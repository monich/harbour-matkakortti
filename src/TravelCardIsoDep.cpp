/*
 * Copyright (C) 2024 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 *  3. Neither the names of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "nfcdc_isodep.h"
#include "nfcdc_tag.h"

#include "TravelCardIsoDep.h"

#include "HarbourDebug.h"

enum tag_events {
    TAG_EVENT_VALID,
    TAG_EVENT_PRESENT,
    TAG_EVENT_COUNT
};

// ==========================================================================
// TravelCardIsoDep::Private
// ==========================================================================

class TravelCardIsoDep::Private
{
public:
    struct Transmit {
        QObject* iObject;
        char* iMethod;

        Transmit(QObject*, const char*);
        ~Transmit();

        static void response(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
        static void free(gpointer);
    };

    Private(const QString&, TravelCardIsoDep*);
    ~Private();

    void startReadingIfReady();
    void readDone();

    static void tagEventHandler(NfcTagClient*, NFC_TAG_PROPERTY, void*);
    static void isoDepEventHandler(NfcIsoDepClient*, NFC_ISODEP_PROPERTY, void*);
    static void tagLockResp(NfcTagClient*, NfcTagClientLock*, const GError*, void*);

public:
    TravelCardIsoDep* iCard;
    NfcTagClient* iTag;
    NfcTagClientLock* iLock;
    NfcIsoDepClient* iIsoDep;
    gulong iTagEventId[TAG_EVENT_COUNT];
    gulong iIsoDepEventId[TAG_EVENT_COUNT];
    GCancellable* iCancel;
};

TravelCardIsoDep::Private::Private(
    const QString& aPath,
    TravelCardIsoDep* aCard) :
    iCard(aCard),
    iTag(Q_NULLPTR),
    iLock(Q_NULLPTR),
    iIsoDep(Q_NULLPTR),
    iCancel(Q_NULLPTR)
{
    memset(iTagEventId, 0, sizeof(iTagEventId));
    memset(iIsoDepEventId, 0, sizeof(iIsoDepEventId));

    QByteArray bytes(aPath.toLatin1());
    const char* path = bytes.constData();

    iTag = nfc_tag_client_new(path);
    iTagEventId[TAG_EVENT_VALID] =
        nfc_tag_client_add_property_handler(iTag,
            NFC_TAG_PROPERTY_VALID, tagEventHandler, this);
    iTagEventId[TAG_EVENT_PRESENT] =
        nfc_tag_client_add_property_handler(iTag,
            NFC_TAG_PROPERTY_PRESENT, tagEventHandler, this);

    iIsoDep = nfc_isodep_client_new(path);
    iIsoDepEventId[TAG_EVENT_VALID] =
        nfc_isodep_client_add_property_handler(iIsoDep,
            NFC_ISODEP_PROPERTY_VALID, isoDepEventHandler, this);
    iIsoDepEventId[TAG_EVENT_PRESENT] =
        nfc_isodep_client_add_property_handler(iIsoDep,
            NFC_ISODEP_PROPERTY_PRESENT, isoDepEventHandler, this);
}


TravelCardIsoDep::Private::~Private()
{
    readDone();
    nfc_isodep_client_unref(iIsoDep);
    nfc_tag_client_unref(iTag);
}

void
TravelCardIsoDep::Private::readDone()
{
    nfc_isodep_client_remove_all_handlers(iIsoDep, iIsoDepEventId);
    nfc_tag_client_remove_all_handlers(iTag, iTagEventId);
    if (iCancel) {
        g_cancellable_cancel(iCancel);
        g_object_unref(iCancel);
        iCancel = Q_NULLPTR;
    }
    if (iLock) {
        nfc_tag_client_lock_unref(iLock);
        iLock = Q_NULLPTR;
    }
}

void
TravelCardIsoDep::Private::startReadingIfReady()
{
    if (iIsoDep->valid && iTag->valid) {
        if (iTag->present && iIsoDep->present && !iCancel) {
            iCancel = g_cancellable_new();
            nfc_tag_client_acquire_lock(iTag, TRUE, iCancel, tagLockResp,
                this, Q_NULLPTR);
        } else if (!iIsoDep->present) {
            // Not an ISO-DEP card
            iCard->failure(UnsupportedCard);
        }
    }
}

/* static */
void
TravelCardIsoDep::Private::tagLockResp(
    NfcTagClient*,
    NfcTagClientLock* aLock,
    const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;
    TravelCardIsoDep* card = self->iCard;

    HASSERT(!self->iLock);
    if (aLock) {
        self->iLock = nfc_tag_client_lock_ref(aLock);
        card->startIo();
    } else {
        HWARN("Failed to lock the tag:" << aError->message);
        card->failure(LockFailure);
    }
}

/* static */
void
TravelCardIsoDep::Private::isoDepEventHandler(
    NfcIsoDepClient*,
    NFC_ISODEP_PROPERTY,
    void* aPrivate)
{
    ((Private*)aPrivate)->startReadingIfReady();
}

/* static */
void
TravelCardIsoDep::Private::tagEventHandler(
    NfcTagClient*,
    NFC_TAG_PROPERTY,
    void* aPrivate)
{
    ((Private*)aPrivate)->startReadingIfReady();
}

// ==========================================================================
// TravelCardIsoDep::Private::Transmit
// ==========================================================================

TravelCardIsoDep::Private::Transmit::Transmit(
    QObject* aObject,
    const char* aMethod) :
    iObject(aObject),
    iMethod(g_strdup(aMethod))
{
}

TravelCardIsoDep::Private::Transmit::~Transmit()
{
    g_free(iMethod);
}

/* static */
void
TravelCardIsoDep::Private::Transmit::response(
    NfcIsoDepClient*,
    const GUtilData* aData,
    guint aSw,
    const GError* aError,
    void* aTransmitData)
{
    Transmit* self = (Transmit*)aTransmitData;

    QMetaObject::invokeMethod(self->iObject, self->iMethod,
                              Q_ARG(const GUtilData*, aData),
                              Q_ARG(uint, aSw),
                              Q_ARG(const GError*, aError));
}

/* static */
void
TravelCardIsoDep::Private::Transmit::free(
    gpointer aTransmitData)
{
    delete (Transmit*)aTransmitData;
}

// ==========================================================================
// TravelCardIsoDep
// ==========================================================================

TravelCardIsoDep::TravelCardIsoDep(
    QString aPath,
    QObject* aParent) :
    TravelCardImpl(aParent),
    iPrivate(new Private(aPath, this))
{
}

TravelCardIsoDep::~TravelCardIsoDep()
{
    delete iPrivate;
}

bool
TravelCardIsoDep::transmit(
    const NfcIsoDepApdu* aApdu,
    QObject* aObject,
    const char* aMethod)
{
    Private::Transmit* tx = new Private::Transmit(aObject, aMethod);
    return nfc_isodep_client_transmit(iPrivate->iIsoDep, aApdu, iPrivate->iCancel,
        Private::Transmit::response, tx, Private::Transmit::free);
}

void
TravelCardIsoDep::success(
    QString aUrl,
    QVariantMap aInfo)
{
    HDEBUG("Read done");
    iPrivate->readDone();
    Q_EMIT readDone(aUrl, aInfo);
}

void
TravelCardIsoDep::failure(Failure)
{
    HDEBUG("Read failed");
    iPrivate->readDone();
    Q_EMIT readFailed();
}

void
TravelCardIsoDep::startReading()
{
    iPrivate->startReadingIfReady();
}

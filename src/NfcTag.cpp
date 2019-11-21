/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019 Slava Monich <slava@monich.com>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
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

#include "nfcdc_tag.h"

#include <gutil_strv.h>

#include "NfcTag.h"

#include "HarbourDebug.h"

enum tag_events {
    TAG_EVENT_VALID,
    TAG_EVENT_PRESENT,
    TAG_EVENT_INTERFACES,
    TAG_EVENT_COUNT
};

// ==========================================================================
// NfcTag::Private
// ==========================================================================

class NfcTag::Private {
public:

    Private(NfcTag* aParent);
    ~Private();

    void setPath(const char* aPath);
    bool updateType();
    void updateTypeAndEmitSignal();

    void emitValidChanged();
    void emitPresentChanged();
    void emitTypeChanged();

    static void validChanged(NfcTagClient*, NFC_TAG_PROPERTY, void*);
    static void presentChanged(NfcTagClient*, NFC_TAG_PROPERTY, void*);
    static void interfacesChanged(NfcTagClient*, NFC_TAG_PROPERTY, void*);

public:
    NfcTag* iParent;
    NfcTagClient* iTag;
    gulong iTagEventId[TAG_EVENT_COUNT];
    Type iType;
};

NfcTag::Private::Private(NfcTag* aParent) :
    iParent(aParent),
    iTag(NULL),
    iType(Unknown)
{
    memset(iTagEventId, 0, sizeof(iTagEventId));
}

NfcTag::Private::~Private()
{
    nfc_tag_client_remove_all_handlers(iTag, iTagEventId);
    nfc_tag_client_unref(iTag);
}

void NfcTag::Private::setPath(const char* aPath)
{
    nfc_tag_client_remove_all_handlers(iTag, iTagEventId);
    nfc_tag_client_unref(iTag);
    if (aPath) {
        iTag = nfc_tag_client_new(aPath);
        iTagEventId[TAG_EVENT_VALID] =
            nfc_tag_client_add_property_handler(iTag,
                NFC_TAG_PROPERTY_VALID, validChanged, this);
        iTagEventId[TAG_EVENT_PRESENT] =
            nfc_tag_client_add_property_handler(iTag,
                NFC_TAG_PROPERTY_PRESENT, presentChanged, this);
        iTagEventId[TAG_EVENT_INTERFACES] =
            nfc_tag_client_add_property_handler(iTag,
                NFC_TAG_PROPERTY_INTERFACES, interfacesChanged, this);
    } else {
        iTag = NULL;
    }
    updateType();
}

bool NfcTag::Private::updateType()
{
    Type type = Unknown;
    if (iTag) {
        if (gutil_strv_contains(iTag->interfaces,
            NFC_TAG_INTERFACE_ISODEP)) {
            type = IsoDep;
        } else if (gutil_strv_contains(iTag->interfaces,
            NFC_TAG_INTERFACE_TYPE2)) {
            type = Type2;
        }
    }
    if (iType != type) {
        iType = type;
        return true;
    } else {
        return false;
    }
}

// Qt calls from glib callbacks better go through QMetaObject::invokeMethod
// See https://bugreports.qt.io/browse/QTBUG-18434 for details

inline void NfcTag::Private::emitValidChanged()
{
    QMetaObject::invokeMethod(iParent, "validChanged");
}

inline void NfcTag::Private::emitPresentChanged()
{
    QMetaObject::invokeMethod(iParent, "presentChanged");
}

inline void NfcTag::Private::emitTypeChanged()
{
    QMetaObject::invokeMethod(iParent, "typeChanged");
}

inline void NfcTag::Private::updateTypeAndEmitSignal()
{
    if (updateType()) {
        emitTypeChanged();
    }
}

void NfcTag::Private::validChanged(NfcTagClient* aTag,
    NFC_TAG_PROPERTY, void* aPrivate)
{
    Private* self = (Private*)aPrivate;
    if (aTag->valid) {
        self->updateTypeAndEmitSignal();
        self->emitValidChanged();
    } else {
        self->emitValidChanged();
        self->updateTypeAndEmitSignal();
    }
}

void NfcTag::Private::presentChanged(NfcTagClient*,
    NFC_TAG_PROPERTY, void* aPrivate)
{
    Private* self = (Private*)aPrivate;
    self->updateTypeAndEmitSignal();
    self->emitPresentChanged();
}

void NfcTag::Private::interfacesChanged(NfcTagClient*,
    NFC_TAG_PROPERTY, void* aPrivate)
{
    ((Private*)aPrivate)->updateTypeAndEmitSignal();
}

// ==========================================================================
// NfcTag
// ==========================================================================

NfcTag::NfcTag(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

NfcTag::~NfcTag()
{
    delete iPrivate;
}

void NfcTag::setPath(QString aPath)
{
    const QString currentPath(path());
    if (currentPath != aPath) {
        const bool wasValid = valid();
        const bool wasPresent = present();
        const Type prevType = type();
        HDEBUG(aPath);
        if (aPath.isEmpty()) {
            iPrivate->setPath(NULL);
        } else {
            QByteArray bytes(aPath.toLatin1());
            iPrivate->setPath(bytes.constData());
        }
        static bool isValid = valid();
        Q_EMIT pathChanged();
        if (wasValid && !isValid) {
            // valid has become false
            Q_EMIT validChanged();
        }
        if (wasPresent != present()) {
            Q_EMIT presentChanged();
        }
        if (prevType != type()) {
            Q_EMIT typeChanged();
        }
        if (isValid && !wasValid) {
            // valid has become true
            Q_EMIT validChanged();
        }
    }
}

QString NfcTag::path() const
{
    return iPrivate->iTag ? QString(iPrivate->iTag->path) : QString();
}

bool NfcTag::valid() const
{
    return iPrivate->iTag && iPrivate->iTag->valid;
}

bool NfcTag::present() const
{
    return iPrivate->iTag && iPrivate->iTag->present;
}

NfcTag::Type NfcTag::type() const
{
    return iPrivate->iType;
}

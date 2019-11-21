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

#include "nfcdc_isodep.h"

#include "NfcIsoDep.h"

#include "HarbourDebug.h"

enum isodep_events {
    ISODEP_EVENT_VALID,
    ISODEP_EVENT_PRESENT,
    ISODEP_EVENT_COUNT
};

// ==========================================================================
// NfcIsoDep::Private
// ==========================================================================

class NfcIsoDep::Private {
public:

    Private(NfcIsoDep* aParent);
    ~Private();

    void setPath(const char* aPath);

    static void validChanged(NfcIsoDepClient* aIsoDep,
        NFC_ISODEP_PROPERTY aProperty, void* aTarget);
    static void presentChanged(NfcIsoDepClient* aIsoDep,
        NFC_ISODEP_PROPERTY aProperty, void* aTarget);

public:
    NfcIsoDep* iParent;
    NfcIsoDepClient* iIsoDep;
    gulong iIsoDepEventId[ISODEP_EVENT_COUNT];
};

NfcIsoDep::Private::Private(NfcIsoDep* aParent) :
    iParent(aParent),
    iIsoDep(NULL)
{
    memset(iIsoDepEventId, 0, sizeof(iIsoDepEventId));
}

NfcIsoDep::Private::~Private()
{
    nfc_isodep_client_remove_all_handlers(iIsoDep, iIsoDepEventId);
    nfc_isodep_client_unref(iIsoDep);
}

void NfcIsoDep::Private::setPath(const char* aPath)
{
    nfc_isodep_client_remove_all_handlers(iIsoDep, iIsoDepEventId);
    nfc_isodep_client_unref(iIsoDep);
    if (aPath) {
        iIsoDep = nfc_isodep_client_new(aPath);
        iIsoDepEventId[ISODEP_EVENT_VALID] =
            nfc_isodep_client_add_property_handler(iIsoDep,
                NFC_ISODEP_PROPERTY_VALID, validChanged, iParent);
        iIsoDepEventId[ISODEP_EVENT_PRESENT] =
            nfc_isodep_client_add_property_handler(iIsoDep,
                NFC_ISODEP_PROPERTY_PRESENT, presentChanged, iParent);
    } else {
        iIsoDep = NULL;
    }
}

// Qt calls from glib callbacks better go through QMetaObject::invokeMethod
// See https://bugreports.qt.io/browse/QTBUG-18434 for details

void NfcIsoDep::Private::validChanged(NfcIsoDepClient*,
    NFC_ISODEP_PROPERTY, void* aTarget)
{
    QMetaObject::invokeMethod((QObject*)aTarget, "validChanged");
}

void NfcIsoDep::Private::presentChanged(NfcIsoDepClient*,
    NFC_ISODEP_PROPERTY, void* aTarget)
{
    QMetaObject::invokeMethod((QObject*)aTarget, "presentChanged");
}

// ==========================================================================
// NfcIsoDep
// ==========================================================================

NfcIsoDep::NfcIsoDep(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

NfcIsoDep::~NfcIsoDep()
{
    delete iPrivate;
}

bool NfcIsoDep::valid() const
{
    return iPrivate->iIsoDep && iPrivate->iIsoDep->valid;
}

bool NfcIsoDep::present() const
{
    return iPrivate->iIsoDep && iPrivate->iIsoDep->present;
}

QString NfcIsoDep::path() const
{
    return iPrivate->iIsoDep ? QString(iPrivate->iIsoDep->path) : QString();
}

void NfcIsoDep::setPath(QString aPath)
{
    const QString currentPath(path());
    if (currentPath != aPath) {
        const bool wasValid = valid();
        const bool wasPresent = present();
        HDEBUG(aPath);
        if (aPath.isEmpty()) {
            iPrivate->setPath(NULL);
        } else {
            QByteArray bytes(aPath.toLatin1());
            iPrivate->setPath(bytes.constData());
        }
        Q_EMIT pathChanged();
        if (wasValid != valid()) {
            Q_EMIT validChanged();
        }
        if (wasPresent != present()) {
            Q_EMIT presentChanged();
        }
    }
}

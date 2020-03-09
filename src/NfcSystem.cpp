/*
 * Copyright (C) 2019-2020 Jolla Ltd.
 * Copyright (C) 2019-2020 Slava Monich <slava@monich.com>
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

#include "nfcdc_daemon.h"

#include "NfcSystem.h"

#include "HarbourDebug.h"

G_STATIC_ASSERT(NfcSystem::Version_1_0_26 == NFC_DAEMON_VERSION(1,0,26));

// ==========================================================================
// NfcSystem::Private
// ==========================================================================

class NfcSystem::Private {
public:

    Private(NfcSystem* aParent);
    ~Private();

    static const char* SIGNAL_NAME[];
    static void propertyChanged(NfcDaemonClient* aDaemon,
        NFC_DAEMON_PROPERTY aProperty, void* aTarget);

public:
    NfcDaemonClient* iDaemon;
    gulong iDaemonEventId[4]; // Must match number of non-NULLs below:
};

const char* NfcSystem::Private::SIGNAL_NAME[] = {
    NULL,               // NFC_DAEMON_PROPERTY_ANY
    "validChanged",     // NFC_DAEMON_PROPERTY_VALID
    "presentChanged",   // NFC_DAEMON_PROPERTY_PRESENT
    NULL,               // NFC_DAEMON_PROPERTY_ERROR
    "enabledChanged",   // NFC_DAEMON_PROPERTY_ENABLED
    NULL,               // NFC_DAEMON_PROPERTY_ADAPTERS
    "versionChanged"    // NFC_DAEMON_PROPERTY_VERSION
};

NfcSystem::Private::Private(NfcSystem* aParent) :
    iDaemon(nfc_daemon_client_new())
{
    int k = 0;
    for (int i = 0; i < NFC_DAEMON_PROPERTY_COUNT; i++) {
        if (SIGNAL_NAME[i]) {
            iDaemonEventId[k++] =
                nfc_daemon_client_add_property_handler(iDaemon,
                    (NFC_DAEMON_PROPERTY)i, propertyChanged, aParent);
        }
    }
    HASSERT(k == G_N_ELEMENTS(iDaemonEventId));
    G_STATIC_ASSERT(G_N_ELEMENTS(NfcSystem::Private::SIGNAL_NAME) ==
        NFC_DAEMON_PROPERTY_COUNT);
}

NfcSystem::Private::~Private()
{
    nfc_daemon_client_remove_all_handlers(iDaemon, iDaemonEventId);
    nfc_daemon_client_unref(iDaemon);
}

// Qt calls from glib callbacks better go through QMetaObject::invokeMethod
// See https://bugreports.qt.io/browse/QTBUG-18434 for details

void NfcSystem::Private::propertyChanged(NfcDaemonClient*,
    NFC_DAEMON_PROPERTY aProperty, void* aTarget)
{
    QMetaObject::invokeMethod((QObject*)aTarget, SIGNAL_NAME[aProperty]);
}

// ==========================================================================
// NfcSystem
// ==========================================================================

NfcSystem::NfcSystem(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

NfcSystem::~NfcSystem()
{
    delete iPrivate;
}

QObject* NfcSystem::createSingleton(QQmlEngine* aEngine, QJSEngine* aScript)
{
    return new NfcSystem;
}

bool NfcSystem::valid() const
{
    return iPrivate->iDaemon->valid;
}

bool NfcSystem::present() const
{
    return iPrivate->iDaemon->present;
}

bool NfcSystem::enabled() const
{
    return iPrivate->iDaemon->enabled;
}

int NfcSystem::version() const
{
    return iPrivate->iDaemon->version;
}

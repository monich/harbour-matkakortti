/*
 * Copyright (C) 2020-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2020 Jolla Ltd.
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

#include "NysseCardBalance.h"
#include "Util.h"

#include "HarbourDebug.h"

// s(SignalName,signalName)
#define QUEUED_SIGNALS(s) \
    s(Data,data) \
    s(Valid,valid) \
    s(Balance,balance)

// ==========================================================================
// NysseCardBalance::Private
// ==========================================================================

class NysseCardBalance::Private
{
public:
    enum Signal {
        #define SIGNAL_ENUM_(Name,name) Signal##Name##Changed,
        QUEUED_SIGNALS(SIGNAL_ENUM_)
        #undef SIGNAL_ENUM_
        SignalCount
    };

    typedef void (NysseCardBalance::*SignalEmitter)();
    typedef uint SignalMask;

    Private();

    void queueSignal(Signal);
    void emitQueuedSignals(NysseCardBalance*);
    void setHexData(QString aHexData);

public:
    SignalMask iQueuedSignals;
    Signal iFirstQueuedSignal;
    QString iHexData;
    bool iValid;
    uint iBalance;
};

NysseCardBalance::Private::Private() :
    iQueuedSignals(0),
    iFirstQueuedSignal(SignalCount),
    iValid(false),
    iBalance(0)
{}

void
NysseCardBalance::Private::queueSignal(
    Signal aSignal)
{
    if (aSignal >= 0 && aSignal < SignalCount) {
        const SignalMask signalBit = (SignalMask(1) << aSignal);
        if (iQueuedSignals) {
            iQueuedSignals |= signalBit;
            if (iFirstQueuedSignal > aSignal) {
                iFirstQueuedSignal = aSignal;
            }
        } else {
            iQueuedSignals = signalBit;
            iFirstQueuedSignal = aSignal;
        }
    }
}

void
NysseCardBalance::Private::emitQueuedSignals(
    NysseCardBalance* aObject)
{
    static const SignalEmitter emitSignal [] = {
        #define SIGNAL_EMITTER_(Name,name) &NysseCardBalance::name##Changed,
        QUEUED_SIGNALS(SIGNAL_EMITTER_)
        #undef SIGNAL_EMITTER_
    };
    Q_STATIC_ASSERT(G_N_ELEMENTS(emitSignal) == SignalCount);
    if (iQueuedSignals) {
        // Reset first queued signal before emitting the signals.
        // Signal handlers may emit new signals.
        uint i = iFirstQueuedSignal;
        iFirstQueuedSignal = SignalCount;
        for (; i < SignalCount && iQueuedSignals; i++) {
            const SignalMask signalBit = (SignalMask(1) << i);
            if (iQueuedSignals & signalBit) {
                iQueuedSignals &= ~signalBit;
                Q_EMIT (aObject->*(emitSignal[i]))();
            }
        }
    }
}

void
NysseCardBalance::Private::setHexData(
    QString aHexData)
{
    if (iHexData != aHexData) {
        iHexData = aHexData;
        HDEBUG(qPrintable(iHexData));
        queueSignal(SignalDataChanged);

        uint balance = 0;
        bool valid = false;
        const QByteArray bytes(QByteArray::fromHex(aHexData.toLatin1()));
        if (bytes.size() == 4) {
            const uchar* data = (const uchar*) bytes.constData();
            balance = Util::uint32le(data);
            HDEBUG("  Balance =" << balance);
            valid = true;
        }

        if (iBalance != balance) {
            iBalance = balance;
            queueSignal(SignalBalanceChanged);
        }
        if (iValid != valid) {
            iValid = valid;
            queueSignal(SignalValidChanged);
        }
    }
}

// ==========================================================================
// NysseCardBalance
// ==========================================================================

NysseCardBalance::NysseCardBalance(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private)
{
}

NysseCardBalance::~NysseCardBalance()
{
    delete iPrivate;
}

QString
NysseCardBalance::data() const
{
    return iPrivate->iHexData;
}

void
NysseCardBalance::setData(
    QString aData)
{
    iPrivate->setHexData(aData.toLower());
    iPrivate->emitQueuedSignals(this);
}

bool
NysseCardBalance::valid() const
{
    return iPrivate->iValid;
}

uint
NysseCardBalance::balance() const
{
    return iPrivate->iBalance;
}

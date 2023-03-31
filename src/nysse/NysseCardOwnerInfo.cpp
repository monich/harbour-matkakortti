/*
 * Copyright (C) 2020-2023 Slava Monich <slava@monich.com>
 * Copyright (C) 2020 Jolla Ltd.
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

#include "NysseCardOwnerInfo.h"
#include "NysseUtil.h"
#include "Util.h"

#include "HarbourDebug.h"

// s(SignalName,signalName)
#define QUEUED_SIGNALS(s) \
    s(Data,data) \
    s(OwnerName,ownerName) \
    s(BirthDate,birthDate)

// ==========================================================================
// NysseCardOwnerInfo::Private
// ==========================================================================

class NysseCardOwnerInfo::Private :
    public QObject
{
    Q_OBJECT

public:
    enum Signal {
#define SIGNAL_ENUM_(Name,name) Signal##Name##Changed,
        QUEUED_SIGNALS(SIGNAL_ENUM_)
#undef  SIGNAL_ENUM_
        SignalCount
    };

    typedef void (NysseCardOwnerInfo::*SignalEmitter)();
    typedef uint SignalMask;

    Private(NysseCardOwnerInfo*);

    void queueSignal(Signal);
    void emitQueuedSignals();

    void updateHexData(const QString);

public:
    SignalMask iQueuedSignals;
    Signal iFirstQueuedSignal;
    QString iHexData;
    QString iOwnerName;
    QDateTime iBirthDate;
};

NysseCardOwnerInfo::Private::Private(
    NysseCardOwnerInfo* aParent) :
    QObject(aParent),
    iQueuedSignals(0),
    iFirstQueuedSignal(SignalCount)
{
}

void
NysseCardOwnerInfo::Private::queueSignal(
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
NysseCardOwnerInfo::Private::emitQueuedSignals()
{
    static const SignalEmitter emitSignal [] = {
#define SIGNAL_EMITTER_(Name,name) &NysseCardOwnerInfo::name##Changed,
        QUEUED_SIGNALS(SIGNAL_EMITTER_)
#undef SIGNAL_EMITTER_
    };
    if (iQueuedSignals) {
        // Reset first queued signal before emitting the signals.
        // Signal handlers may emit new signals.
        uint i = iFirstQueuedSignal;
        iFirstQueuedSignal = SignalCount;
        NysseCardOwnerInfo* obj = qobject_cast<NysseCardOwnerInfo*>(parent());
        for (; i < SignalCount && iQueuedSignals; i++) {
            const SignalMask signalBit = (SignalMask(1) << i);
            if (iQueuedSignals & signalBit) {
                iQueuedSignals &= ~signalBit;
                Q_EMIT (obj->*(emitSignal[i]))();
            }
        }
    }
}

void
NysseCardOwnerInfo::Private::updateHexData(
    const QString aHexData)
{
    // Owner info layout (96 bytes)
    //
    // +=========================================================+
    // | Offset | Size | Description                             |
    // +=========================================================+
    // | 0      | 6    | ??? (05 00 00 00 00 00)                 |
    // | 6      | 24   | Owner's name, padded with zeros         |
    // | 30     | 4    | ???                                     |
    // | 34     | 2    | Birthdate (days since 1 Jan 1900)       |
    // | 36     | 60   | ???                                     |
    // +=========================================================+
    if (iHexData != aHexData) {
        iHexData = aHexData;
        HDEBUG(qPrintable(iHexData));
        queueSignal(SignalDataChanged);

        const QString prevOwnerName(iOwnerName);
        const QDateTime prevBirthDate(iBirthDate);
        const QByteArray data(QByteArray::fromHex(aHexData.toLatin1()));

        if (data.size() >= 36) {
            const uchar* bytes = (const uchar*)data.constData();

            // Owner name is padded with zeros
            const char* name = data.constData() + 6;
            int nameLen = 24;
            while (nameLen > 0 && !name[nameLen - 1]) nameLen--;
            iOwnerName = QString::fromLatin1(name, nameLen);
            HDEBUG("  OwnerName =" << iOwnerName);

            iBirthDate = NysseUtil::toDateTime(Util::uint16le(bytes + 34), 0);
            HDEBUG("  BirthDate =" << iBirthDate.date());
        } else {
            iOwnerName.clear();
            iBirthDate = QDateTime();
        }

        if (prevOwnerName != iOwnerName) {
            queueSignal(SignalOwnerNameChanged);
        }
        if (prevBirthDate != iBirthDate) {
            queueSignal(SignalBirthDateChanged);
        }
    }
}

// ==========================================================================
// NysseCardOwnerInfo
// ==========================================================================

NysseCardOwnerInfo::NysseCardOwnerInfo(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

QString
NysseCardOwnerInfo::data() const
{
    return iPrivate->iHexData;
}

void
NysseCardOwnerInfo::setData(
    const QString aHexData)
{
    iPrivate->updateHexData(aHexData);
    iPrivate->emitQueuedSignals();
}

QString
NysseCardOwnerInfo::ownerName() const
{
    return iPrivate->iOwnerName;
}

QDateTime
NysseCardOwnerInfo::birthDate() const
{
    return iPrivate->iBirthDate;
}

#include "NysseCardOwnerInfo.moc"

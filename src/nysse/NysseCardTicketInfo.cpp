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

#include "gutil_timenotify.h"

#include "NysseCardTicketInfo.h"
#include "NysseUtil.h"
#include "TravelCard.h"
#include "Util.h"

#include "HarbourDebug.h"

// s(SignalName,signalName)
#define QUEUED_SIGNALS(s) \
    s(Data,data) \
    s(Valid,valid) \
    s(DaysRemaining,daysRemaining) \
    s(EndDate,endDate)

// ==========================================================================
// NysseCardTicketInfo::Private
// ==========================================================================

class NysseCardTicketInfo::Private :
    public QObject
{
    Q_OBJECT

public:
    enum Signal {
        #define SIGNAL_ENUM_(Name,name) Signal##Name##Changed,
        QUEUED_SIGNALS(SIGNAL_ENUM_)
        #undef SIGNAL_ENUM_
        SignalCount
    };

    typedef void (NysseCardTicketInfo::*SignalEmitter)();
    typedef uint SignalMask;

    Private(NysseCardTicketInfo*);
    ~Private();

    void queueSignal(Signal);
    void emitQueuedSignals();

    void updateHexData(QString);
    void updateDaysRemaining();
    void scheduleRefreshDaysRemaining();

    static void systemTimeChanged(GUtilTimeNotify*, void*);

public Q_SLOTS:
    void refreshDaysRemaining();

public:
    SignalMask iQueuedSignals;
    Signal iFirstQueuedSignal;
    QString iHexData;
    QDateTime iEndDate;
    bool iValid;
    int iDaysRemaining;
    GUtilTimeNotify* iTimeNotify;
    gulong iTimeNotifyId;
};

NysseCardTicketInfo::Private::Private(
    NysseCardTicketInfo* aParent) :
    QObject(aParent),
    iQueuedSignals(0),
    iFirstQueuedSignal(SignalCount),
    iValid(false),
    iDaysRemaining(TravelCard::PeriodInvalid),
    iTimeNotify(gutil_time_notify_new()),
    iTimeNotifyId(gutil_time_notify_add_handler(iTimeNotify,
        systemTimeChanged, this))
{
}

NysseCardTicketInfo::Private::~Private()
{
    gutil_time_notify_remove_handler(iTimeNotify, iTimeNotifyId);
    gutil_time_notify_unref(iTimeNotify);
}

void
NysseCardTicketInfo::Private::queueSignal(
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
NysseCardTicketInfo::Private::emitQueuedSignals()
{
    static const SignalEmitter emitSignal [] = {
        #define SIGNAL_EMITTER_(Name,name) &NysseCardTicketInfo::name##Changed,
        QUEUED_SIGNALS(SIGNAL_EMITTER_)
        #undef SIGNAL_EMITTER_
    };
    Q_STATIC_ASSERT(G_N_ELEMENTS(emitSignal) == SignalCount);
    if (iQueuedSignals) {
        // Reset first queued signal before emitting the signals.
        // Signal handlers may emit new signals.
        uint i = iFirstQueuedSignal;
        iFirstQueuedSignal = SignalCount;
        NysseCardTicketInfo* obj = qobject_cast<NysseCardTicketInfo*>(parent());
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
NysseCardTicketInfo::Private::updateHexData(
    const QString aHexData)
{
    const QByteArray bytes(QByteArray::fromHex(aHexData.toLatin1()));

    iHexData = aHexData;
    HDEBUG(qPrintable(iHexData));

    // Season pass info contains two 48 bytes blocks.
    // Block layout:
    //
    // +=========================================================+
    // | Offset | Size | Description                             |
    // +=========================================================+
    // | 0      | 1    | Block id                                |
    // | 1      | 5    | ??? (usually 0f 03 00 00 00)            |
    // | 6      | 1    | Record type?                            |
    // |        |      +-----------------------------------------+
    // |        |      | 0x00 | Empty record                     |
    // |        |      | 0x03 | Subscription (period ticket)     |
    // |        |      | 0x3f | ???                              |
    // |        |      +-----------------------------------------+
    // | 10     | 2    | Subscription end date (record type 3)   |
    // +=========================================================+
    if (bytes.size() >= 96) {
        const uchar* data = (const uchar*) bytes.constData();
        const uint id1 = data[0];
        const uint id2 = data[48];
        const uint off = (id1 > id2 && (id1 - id2) <= 128) ? 0 : 48;

        HDEBUG("Block ids" << hex << id1 << "and" << id2 << "using the" <<
            (off ? "second" : "first") << "one");

        bool valid = false;
        if (data[6] == 3) {
            const QDateTime endDate(NysseUtil::toDateTime(Util::uint16be(data + 10)));
            HDEBUG("  EndDate =" << endDate);
            if (iEndDate != endDate) {
                iEndDate = endDate;
                queueSignal(SignalEndDateChanged);
            }
            valid = true;
        }

        HDEBUG("  Valid =" << valid);
        if (iValid != valid) {
            iValid = valid;
            queueSignal(SignalValidChanged);
        }
    }

    updateDaysRemaining();
    scheduleRefreshDaysRemaining();
}

void
NysseCardTicketInfo::Private::systemTimeChanged(
    GUtilTimeNotify*,
    void* aPrivate)
{
    HDEBUG("System time changed");
    QTimer::singleShot(0, (Private*)aPrivate, SLOT(refreshDaysRemaining()));
}

void
NysseCardTicketInfo::Private::refreshDaysRemaining()
{
    updateDaysRemaining();
    emitQueuedSignals();
    scheduleRefreshDaysRemaining();
}

void
NysseCardTicketInfo::Private::scheduleRefreshDaysRemaining()
{
    const QDateTime now(Util::currentTimeInFinland());
    const QDate today = now.date();
    const QDateTime nextMidnight(today.addDays(1), QTime(0,0), Util::FINLAND_TIMEZONE);
    HDEBUG(now.toString("dd.MM.yyyy hh:mm:ss") << now.secsTo(nextMidnight) << "sec until midnight");
    QTimer::singleShot(now.msecsTo(nextMidnight) + 1000, this, SLOT(refreshDaysRemaining()));
}

void
NysseCardTicketInfo::Private::updateDaysRemaining()
{
    const int prevDaysRemaining = iDaysRemaining;
    if (iValid) {
        const QDateTime now = QDateTime::currentDateTime();
        const QDate today = now.date();
        const QDate lastDay(iEndDate.date());
        if (today > lastDay) {
            iDaysRemaining = TravelCard::PeriodEnded;
        } else {
            iDaysRemaining = today.daysTo(lastDay) + 1;
        }
    } else {
        iDaysRemaining = TravelCard::PeriodInvalid;
    }
    if (prevDaysRemaining != iDaysRemaining) {
        queueSignal(SignalDaysRemainingChanged);
    }
}

// ==========================================================================
// NysseCardTicketInfo
// ==========================================================================

NysseCardTicketInfo::NysseCardTicketInfo(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

NysseCardTicketInfo::~NysseCardTicketInfo()
{
    delete iPrivate;
}

const QString
NysseCardTicketInfo::data() const
{
    return iPrivate->iHexData;
}

void
NysseCardTicketInfo::setData(
    const QString aData)
{
    const QString data(aData.toLower());
    if (iPrivate->iHexData != data) {
        iPrivate->updateHexData(data);
        iPrivate->queueSignal(Private::SignalDataChanged);
        iPrivate->emitQueuedSignals();
    }
}

bool
NysseCardTicketInfo::valid() const
{
    return iPrivate->iValid;
}

int
NysseCardTicketInfo::daysRemaining() const
{
    return iPrivate->iDaysRemaining;
}

QDateTime
NysseCardTicketInfo::endDate() const
{
    return iPrivate->iEndDate;
}

#include "NysseCardTicketInfo.moc"

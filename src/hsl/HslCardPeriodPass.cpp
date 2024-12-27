/*
 * Copyright (C) 2019-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2019-2020 Jolla Ltd.
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

#include "HslCardPeriodPass.h"
#include "TravelCard.h"
#include "Util.h"

#include <gutil_timenotify.h>

#include "HarbourDebug.h"

// ==========================================================================
// HslCardPeriodPass::Types
// ==========================================================================

// s(SignalName,signalName)
#define QUEUED_SIGNALS(s) \
    s(Data,data) \
    s(EffectiveDaysRemaining,effectiveDaysRemaining) \
    s(EffectiveEndDate,effectiveEndDate) \
    s(PeriodValid1,periodValid1) \
    s(PeriodPrice1,periodPrice1) \
    s(PeriodDays1,periodDays1) \
    s(PeriodDaysRemaining1,periodDaysRemaining1) \
    s(ValidityArea1,validityArea1) \
    s(PeriodStartDate1,periodStartDate1) \
    s(PeriodEndDate1,periodEndDate1) \
    s(LoadingTime1,loadingTime1) \
    s(PeriodValid2,periodValid2) \
    s(PeriodPrice2,periodPrice2) \
    s(PeriodDays2,periodDays2) \
    s(PeriodDaysRemaining2,periodDaysRemaining2) \
    s(ValidityArea2,validityArea2) \
    s(PeriodStartDate2,periodStartDate2) \
    s(PeriodEndDate2,periodEndDate2) \
    s(LoadingTime2,loadingTime2)

class HslCardPeriodPass::Types
{
public:
    enum Signal {
        #define SIGNAL_ENUM_(Name,name) Signal##Name##Changed,
        QUEUED_SIGNALS(SIGNAL_ENUM_)
        #undef SIGNAL_ENUM_
        SignalCount
    };

    typedef QVector<Signal> Signals;
    typedef void (HslCardPeriodPass::*SignalEmitter)();
    typedef uint SignalMask;

    struct PeriodPassSignals {
        Signal periodValid;
        Signal periodPrice;
        Signal periodDays;
        Signal periodDaysRemaining;
        Signal validityArea;
        Signal periodStartDate;
        Signal periodEndDate;
        Signal loadingTime;
    };

    static const PeriodPassSignals PERIOD_PASS_SIGNALS_1;
    static const PeriodPassSignals PERIOD_PASS_SIGNALS_2;
};

const HslCardPeriodPass::Types::PeriodPassSignals
HslCardPeriodPass::Types::PERIOD_PASS_SIGNALS_1 = {
    SignalPeriodValid1Changed,
    SignalPeriodPrice1Changed,
    SignalPeriodDays1Changed,
    SignalPeriodDaysRemaining1Changed,
    SignalValidityArea1Changed,
    SignalPeriodStartDate1Changed,
    SignalPeriodEndDate1Changed,
    SignalLoadingTime1Changed
};

const HslCardPeriodPass::Types::PeriodPassSignals
HslCardPeriodPass::Types::PERIOD_PASS_SIGNALS_2 = {
    SignalPeriodValid2Changed,
    SignalPeriodPrice2Changed,
    SignalPeriodDays2Changed,
    SignalPeriodDaysRemaining2Changed,
    SignalValidityArea2Changed,
    SignalPeriodStartDate2Changed,
    SignalPeriodEndDate2Changed,
    SignalLoadingTime2Changed
};

// ==========================================================================
// HslCardPeriodPass::PeriodPass
// ==========================================================================

class HslCardPeriodPass::PeriodPass :
    public HslCardPeriodPass::Types
{
public:
    PeriodPass(const PeriodPassSignals*);

    Signals reset();
    Signals update(const HslArea&, const QDateTime&, const QDateTime&,
        const QDateTime& aLoadingTime = QDateTime(), int aPrice = 0);
    bool updateDaysRemaining();

public:
    const PeriodPassSignals* iSignals;
    bool iValid;
    HslArea iValidityArea;
    QDateTime iStartDate;
    QDateTime iEndDate;
    QDateTime iLoadingTime;
    int iLoadedDays;
    int iPrice;
    int iDaysRemaining;
};

HslCardPeriodPass::PeriodPass::PeriodPass(
    const PeriodPassSignals* aSignals) :
    iSignals(aSignals),
    iValid(false),
    iLoadedDays(0),
    iPrice(0),
    iDaysRemaining(0)
{
}

HslCardPeriodPass::Types::Signals
HslCardPeriodPass::PeriodPass::reset()
{
    const QDateTime invalid;
    return update(HslArea(), invalid, invalid, invalid, 0);
}

HslCardPeriodPass::Types::Signals
HslCardPeriodPass::PeriodPass::update(
    const HslArea& aValidityArea,
    const QDateTime& aStartDate,
    const QDateTime& aEndDate,
    const QDateTime& aLoadingTime,
    int aPrice)
{
    const QDate firstDay(aStartDate.date());
    const QDate lastDay(aEndDate.date());
    const bool valid = HslData::isValidPeriod(firstDay, lastDay);
    const int days = (valid ? (firstDay.daysTo(lastDay) + 1) : 0);
    Signals changes;

    if (iValid != valid) {
        iValid = valid;
        changes.append(iSignals->periodValid);
    }

    if (iLoadedDays != days) {
        iLoadedDays = days;
        changes.append(iSignals->periodDays);
    }

    if (iStartDate != aStartDate) {
        iStartDate = aStartDate;
        changes.append(iSignals->periodStartDate);
    }

    if (iEndDate != aEndDate) {
        iEndDate = aEndDate;
        changes.append(iSignals->periodEndDate);
    }

    if (iLoadingTime != aLoadingTime) {
        iLoadingTime = aLoadingTime;
        changes.append(iSignals->loadingTime);
    }

    if (iPrice != aPrice) {
        iPrice = aPrice;
        changes.append(iSignals->periodPrice);
    }

    if (!iValidityArea.equals(aValidityArea)) {
        iValidityArea = aValidityArea;
        changes.append(iSignals->validityArea);
    }

    if (updateDaysRemaining()) {
        changes.append(iSignals->periodDaysRemaining);
    }

    return changes;
}

bool
HslCardPeriodPass::PeriodPass::updateDaysRemaining()
{
    int daysRemaining;
    const QDate firstDay(iStartDate.date());
    const QDate lastDay(iEndDate.date());

    if (HslData::isValidPeriod(firstDay, lastDay)) {
        const QDateTime now(Util::currentTimeInFinland());
        const QDate today(now.date());

        if (today < firstDay) {
            daysRemaining = TravelCard::PeriodNotYetStarted;
        } else if (today > lastDay) {
            daysRemaining = TravelCard::PeriodEnded;
        } else {
            daysRemaining = today.daysTo(lastDay) + 1;
        }
    } else {
        daysRemaining = TravelCard::PeriodInvalid;
    }

    if (iDaysRemaining != daysRemaining) {
        iDaysRemaining = daysRemaining;
        return true;
    } else {
        return false;
    }
}

// ==========================================================================
// HslCardPeriodPass::Private
// ==========================================================================

class HslCardPeriodPass::Private :
    public QObject,
    public HslCardPeriodPass::Types
{
    Q_OBJECT

public:
    Private(HslCardPeriodPass*);
    ~Private();

    HslCardPeriodPass* parentObject();
    void queueSignal(Signal aSignal);
    void queueSignals(const Signals& aSignals);
    void emitQueuedSignals();

    void updateHexData(QString);
    void updatePeriods();
    void scheduleRefreshPeriods();

    static void systemTimeChanged(GUtilTimeNotify*, void*);

public Q_SLOTS:
    void refreshPeriods();

public:
    SignalMask iQueuedSignals;
    Signal iFirstQueuedSignal;
    QString iHexData;
    HslArea iValidityArea1;
    HslArea iValidityArea2;
    QDate iPeriodStartDate1;
    QDate iPeriodStartDate2;
    QDate iPeriodEndDate1;
    QDate iPeriodEndDate2;
    QDateTime iLastLoadingTime;
    int iLatestPeriodPrice;
    int iEffectiveDaysRemaining;
    QDateTime iEffectiveEndDate;
    PeriodPass iPeriodPass1;
    PeriodPass iPeriodPass2;
    GUtilTimeNotify* iTimeNotify;
    gulong iTimeNotifyId;
};

HslCardPeriodPass::Private::Private(
    HslCardPeriodPass* aParent) :
    QObject(aParent),
    iQueuedSignals(0),
    iFirstQueuedSignal(SignalCount),
    iLatestPeriodPrice(0),
    iEffectiveDaysRemaining(0),
    iPeriodPass1(&PERIOD_PASS_SIGNALS_1),
    iPeriodPass2(&PERIOD_PASS_SIGNALS_2),
    iTimeNotify(gutil_time_notify_new()),
    iTimeNotifyId(gutil_time_notify_add_handler(iTimeNotify,
        systemTimeChanged, this))
{
}

HslCardPeriodPass::Private::~Private()
{
    gutil_time_notify_remove_handler(iTimeNotify, iTimeNotifyId);
    gutil_time_notify_unref(iTimeNotify);
}

inline
HslCardPeriodPass*
HslCardPeriodPass::Private::parentObject()
{
    return qobject_cast<HslCardPeriodPass*>(parent());
}

void
HslCardPeriodPass::Private::queueSignal(
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
HslCardPeriodPass::Private::queueSignals(
    const Signals& aSignals)
{
    const int n = aSignals.count();
    for (int i = 0; i < n; i++) {
        queueSignal(aSignals.at(i));
    }
}

void
HslCardPeriodPass::Private::emitQueuedSignals()
{
    static const SignalEmitter emitSignal [] = {
        #define SIGNAL_EMITTER_(Name,name) &HslCardPeriodPass::name##Changed,
        QUEUED_SIGNALS(SIGNAL_EMITTER_)
        #undef SIGNAL_EMITTER_
    };
    Q_STATIC_ASSERT(G_N_ELEMENTS(emitSignal) == SignalCount);
    if (iQueuedSignals) {
        // Reset first queued signal before emitting the signals.
        // Signal handlers may emit new signals.
        uint i = iFirstQueuedSignal;
        iFirstQueuedSignal = SignalCount;
        HslCardPeriodPass* obj = parentObject();
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
HslCardPeriodPass::Private::updateHexData(
    const QString aHexData)
{
    const QByteArray hexData(aHexData.toLatin1());
    const QByteArray bytes(QByteArray::fromHex(hexData));

    HDEBUG(hexData.constData());
    iHexData = aHexData;

    if (!bytes.isEmpty()) {
        const GUtilData data = Util::toData(bytes);

        HDEBUG("  ProductCodeType1 =" << getInt(&data, 0, 1));
        HDEBUG("  ProductCode1 =" << getInt(&data, 1, 14));
        iValidityArea1 = getArea(&data, 1, 7, 2, 1);
        HDEBUG("  ValidityAreaType1 =" << getInt(&data, 1, 7, 2));
        HDEBUG("  ValidityArea1 =" << getInt(&data, 2, 1, 6) << iValidityArea1);
        iPeriodStartDate1 = getDate(&data, 2, 7);
        HDEBUG("  PeriodStartDate1 =" << getInt(&data, 2, 7, 14) << iPeriodStartDate1);
        iPeriodEndDate1 = getDate(&data, 4, 5);
        HDEBUG("  PeriodEndDate1 =" << getInt(&data, 4, 5, 14) << iPeriodEndDate1);
        HDEBUG("  ProductCodeType2 =" << getInt(&data, 7, 0, 1));
        HDEBUG("  ProductCode2 =" << getInt(&data, 7, 1, 14));
        iValidityArea2 = getArea(&data, 8, 7, 9, 1);
        HDEBUG("  ValidityAreaType2 =" << getInt(&data,  8, 7, 2));
        HDEBUG("  ValidityArea2 =" << getInt(&data, 9, 1, 6) << iValidityArea2);
        iPeriodStartDate2 = getDate(&data, 9, 7);
        HDEBUG("  PeriodStartDate2 =" << getInt(&data, 9, 7, 14) << iPeriodStartDate2);
        iPeriodEndDate2 = getDate(&data, 11, 5);
        HDEBUG("  PeriodEndDate2 =" << getInt(&data, 11, 5, 14) << iPeriodEndDate2);
        HDEBUG("  ProductCodeType =" << getInt(&data, 14, 0, 1));
        HDEBUG("  ProductCode =" << getInt(&data, 14, 1, 14));
        const QDate loadingDate(getDate(&data, 15, 7));
        const QTime loadingTime(getTime(&data, 17, 5));
        iLastLoadingTime = QDateTime(loadingDate, loadingTime, Util::FINLAND_TIMEZONE);
        HDEBUG("  LoadingDate =" << getInt(&data, 15, 7, 14) << loadingDate);
        HDEBUG("  LoadingTime =" << getInt(&data, 17, 5, 11) << loadingTime);
        HDEBUG("  LoadedPeriodDays =" << getInt(&data, 19, 0, 9));
        iLatestPeriodPrice = getInt(&data, 20, 1, 20);
        HDEBUG("  PriceOfPeriod =" << iLatestPeriodPrice);
        HDEBUG("  LoadingOrganisationID =" << getInt(&data, 22, 5, 14));
        HDEBUG("  LoadingDeviceNumber =" << getInt(&data, 24, 3, 13));
        HDEBUG("  BoardingDate =" << getInt(&data, 26, 0, 14));
        HDEBUG("  BoardingTime =" << getInt(&data, 27, 6, 11));
        HDEBUG("  BoardingVehicle =" << getInt(&data, 29, 1, 14));
        HDEBUG("  BoardingLocationNumberType =" << getInt(&data, 30, 7, 2));
        HDEBUG("  BoardingLocationNumber =" << getInt(&data, 31, 1, 14));
        HDEBUG("  BoardingDirection =" << getInt(&data, 32, 7, 1));
        HDEBUG("  BoardingAreaType =" << getInt(&data, 33, 0, 2));
        HDEBUG("  BoardingArea =" << getInt(&data, 33, 2, 6));
        updatePeriods();
    } else {
        HDEBUG("No valid period pass data");
        queueSignals(iPeriodPass1.reset());
        queueSignals(iPeriodPass2.reset());
    }
    queueSignal(SignalDataChanged);
}

void
HslCardPeriodPass::Private::updatePeriods()
{
    Signals signals1, signals2;
    const bool validPeriod1 = isValidPeriod(iPeriodStartDate1, iPeriodEndDate1);
    const bool validPeriod2 = isValidPeriod(iPeriodStartDate2, iPeriodEndDate2);
    if (validPeriod1) {
        if (validPeriod2) {
            const QDate today(Util::currentTimeInFinland().date());
            const bool currentPeriod1 = (today >= iPeriodStartDate1 &&
                today <= iPeriodEndDate1);
            const bool currentPeriod2 = (today >= iPeriodStartDate2 &&
                today <= iPeriodEndDate2);
            const bool latest1 = (iPeriodEndDate1 >= iPeriodEndDate2);

            HDEBUG("Both periods are valid");
            if (currentPeriod1 == currentPeriod2) {
                // Either both periods are current (which shouldn't happen)
                // or both nor (a typical situation)
                if (latest1) {
                    HDEBUG("Current period 1");
                    signals1 = iPeriodPass1.update(iValidityArea1,
                        startDateTime(iPeriodStartDate1),
                        endDateTime(iPeriodEndDate1),
                        iLastLoadingTime, iLatestPeriodPrice);
                    signals2 = iPeriodPass2.update(iValidityArea2,
                        startDateTime(iPeriodStartDate2),
                        endDateTime(iPeriodEndDate2));
                } else {
                    HDEBUG("Current period 2");
                    signals1 = iPeriodPass1.update(iValidityArea2,
                        startDateTime(iPeriodStartDate2),
                        endDateTime(iPeriodEndDate2),
                        iLastLoadingTime, iLatestPeriodPrice);
                    signals2 = iPeriodPass2.update(iValidityArea1,
                        startDateTime(iPeriodStartDate1),
                        endDateTime(iPeriodEndDate1));
                }
            } else {
                if (currentPeriod1) {
                    // Period 1 is current, show it first
                    HDEBUG("Current period 1");
                    signals1 = iPeriodPass1.update(iValidityArea1,
                        startDateTime(iPeriodStartDate1),
                        endDateTime(iPeriodEndDate1),
                        latest1 ? iLastLoadingTime : QDateTime(),
                        latest1 ? iLatestPeriodPrice : 0);
                    signals2 = iPeriodPass2.update(iValidityArea2,
                        startDateTime(iPeriodStartDate2),
                        endDateTime(iPeriodEndDate2),
                        latest1 ? QDateTime() : iLastLoadingTime,
                        latest1 ? 0 : iLatestPeriodPrice);
                } else {
                    // Period 2 is current, show it first
                    HDEBUG("Current period 2");
                    signals1 = iPeriodPass1.update(iValidityArea2,
                        startDateTime(iPeriodStartDate2),
                        endDateTime(iPeriodEndDate2),
                        latest1 ? QDateTime() : iLastLoadingTime,
                        latest1 ? 0 : iLatestPeriodPrice);
                    signals2 = iPeriodPass2.update(iValidityArea1,
                        startDateTime(iPeriodStartDate1),
                        endDateTime(iPeriodEndDate1),
                        latest1 ? iLastLoadingTime : QDateTime(),
                        latest1 ? iLatestPeriodPrice : 0);
                }
            }
        } else {
            HDEBUG("Only period 1 is valid");
            signals1 = iPeriodPass1.update(iValidityArea1,
                startDateTime(iPeriodStartDate1),
                endDateTime(iPeriodEndDate1),
                iLastLoadingTime, iLatestPeriodPrice);
            signals2 = iPeriodPass2.reset();
        }
    } else if (validPeriod2) {
        HDEBUG("Only period 2 is valid");
        signals1 = iPeriodPass1.update(iValidityArea2,
            startDateTime(iPeriodStartDate2),
            endDateTime(iPeriodEndDate2),
            iLastLoadingTime, iLatestPeriodPrice);
        signals2 = iPeriodPass2.reset();
    } else {
        HDEBUG("No valid period passes");
        signals1 = iPeriodPass1.reset();
        signals2 = iPeriodPass2.reset();
    }
    queueSignals(signals1);
    queueSignals(signals2);

    // Handle consecutive periods

    int daysRemaining = 0;
    QDateTime endDate;

    if (iPeriodPass1.iValid && iPeriodPass1.iDaysRemaining > 0) {
        if (iPeriodPass2.iValid && iPeriodPass2.iLoadedDays > 0 &&
            iPeriodPass1.iEndDate.date().daysTo(iPeriodPass2.iStartDate.date()) == 1) {
            daysRemaining = iPeriodPass1.iDaysRemaining + iPeriodPass2.iLoadedDays;
            endDate = iPeriodPass2.iEndDate;
        } else {
            daysRemaining = iPeriodPass1.iDaysRemaining;
            endDate = iPeriodPass1.iEndDate;
        }
    }

    if (iEffectiveDaysRemaining != daysRemaining) {
        iEffectiveDaysRemaining = daysRemaining;
        queueSignal(SignalEffectiveDaysRemainingChanged);
    }
    if (iEffectiveEndDate != endDate) {
        iEffectiveEndDate = endDate;
        queueSignal(SignalEffectiveEndDateChanged);
    }
}

void
HslCardPeriodPass::Private::systemTimeChanged(
    GUtilTimeNotify*,
    void* aPrivate)
{
    HDEBUG("System time changed");
    QTimer::singleShot(0, (Private*) aPrivate, SLOT(refreshPeriods()));
}

void
HslCardPeriodPass::Private::refreshPeriods()
{
    updatePeriods();
    emitQueuedSignals();
    scheduleRefreshPeriods();
}

void
HslCardPeriodPass::Private::scheduleRefreshPeriods()
{
    const QDateTime now(Util::currentTimeInFinland());
    const QDate today = now.date();
    const QDateTime nextMidnight(today.addDays(1), QTime(0,0), Util::FINLAND_TIMEZONE);
    HDEBUG(now.toString("dd.MM.yyyy hh:mm:ss") << now.secsTo(nextMidnight) << "sec until midnight");
    QTimer::singleShot(now.msecsTo(nextMidnight) + 1000, this,
        SLOT(refreshPeriods()));
}

// ==========================================================================
// HslCardPeriodPass
// ==========================================================================

HslCardPeriodPass::HslCardPeriodPass(
    QObject* aParent) :
    HslData(aParent),
    iPrivate(new Private(this))
{
}

HslCardPeriodPass::~HslCardPeriodPass()
{
    delete iPrivate;
}

const QString
HslCardPeriodPass::data() const
{
    return iPrivate->iHexData;
}

void
HslCardPeriodPass::setData(
    const QString aData)
{
    const QString data(aData.toLower());
    if (iPrivate->iHexData != data) {
        iPrivate->updateHexData(data);
        iPrivate->emitQueuedSignals();
        iPrivate->scheduleRefreshPeriods();
    }
}

int
HslCardPeriodPass::effectiveDaysRemaining() const
{
    return iPrivate->iEffectiveDaysRemaining;
}

QDateTime
HslCardPeriodPass::effectiveEndDate() const
{
    return iPrivate->iEffectiveEndDate;
}

bool
HslCardPeriodPass::periodValid1() const
{
    return iPrivate->iPeriodPass1.iValid;
}

int
HslCardPeriodPass::periodPrice1() const
{
    return iPrivate->iPeriodPass1.iPrice;
}

int
HslCardPeriodPass::periodDays1() const
{
    return iPrivate->iPeriodPass1.iLoadedDays;
}

int
HslCardPeriodPass::periodDaysRemaining1() const
{
    return iPrivate->iPeriodPass1.iDaysRemaining;
}

HslArea
HslCardPeriodPass::validityArea1() const
{
    return iPrivate->iPeriodPass1.iValidityArea;
}

const QString
HslCardPeriodPass::validityAreaName1() const
{
    return iPrivate->iPeriodPass1.iValidityArea.name();
}

QDateTime
HslCardPeriodPass::periodStartDate1() const
{
    return iPrivate->iPeriodPass1.iStartDate;
}

QDateTime
HslCardPeriodPass::periodEndDate1() const
{
    return iPrivate->iPeriodPass1.iEndDate;
}

QDateTime
HslCardPeriodPass::loadingTime1() const
{
    return iPrivate->iPeriodPass1.iLoadingTime;
}

bool
HslCardPeriodPass::periodValid2() const
{
    return iPrivate->iPeriodPass2.iValid;
}

int
HslCardPeriodPass::periodPrice2() const
{
    return iPrivate->iPeriodPass2.iPrice;
}

int
HslCardPeriodPass::periodDays2() const
{
    return iPrivate->iPeriodPass2.iLoadedDays;
}

int
HslCardPeriodPass::periodDaysRemaining2() const
{
    return iPrivate->iPeriodPass2.iDaysRemaining;
}

HslArea
HslCardPeriodPass::validityArea2() const
{
    return iPrivate->iPeriodPass2.iValidityArea;
}

const QString
HslCardPeriodPass::validityAreaName2() const
{
    return iPrivate->iPeriodPass2.iValidityArea.name();
}

QDateTime
HslCardPeriodPass::periodStartDate2() const
{
    return iPrivate->iPeriodPass2.iStartDate;
}

QDateTime
HslCardPeriodPass::periodEndDate2() const
{
    return iPrivate->iPeriodPass2.iEndDate;
}

QDateTime
HslCardPeriodPass::loadingTime2() const
{
    return iPrivate->iPeriodPass2.iLoadingTime;
}

#include "HslCardPeriodPass.moc"

/*
 * Copyright (C) 2019-2020 Jolla Ltd.
 * Copyright (C) 2019-2020 Slava Monich <slava.monich@jolla.com>
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

#include "HslCardPeriodPass.h"
#include "TravelCard.h"
#include "Util.h"

#include <gutil_misc.h>
#include <gutil_timenotify.h>

#include "HarbourDebug.h"

// ==========================================================================
// HslCardPeriodPass::Private
// ==========================================================================

class HslCardPeriodPass::Private {
public:
    Private(HslCardPeriodPass* aPass);
    ~Private();

    void setHexData(QString aHexData);
    void updateDaysRemaining();

    static void systemTimeChanged(GUtilTimeNotify*, void*);

public:
    HslCardPeriodPass* iPass;
    QString iHexData;
    HslArea iValidityArea;
    QDateTime iPeriodStartDate;
    QDateTime iPeriodEndDate;
    QDateTime iLoadingTime;
    int iLoadedPeriodDays;
    int iLoadedPeriodPrice;
    int iDaysRemaining;
    GUtilTimeNotify* iTimeNotify;
    gulong iTimeNotifyId;
};

HslCardPeriodPass::Private::Private(HslCardPeriodPass* aPass) :
    iPass(aPass),
    iLoadedPeriodDays(0),
    iLoadedPeriodPrice(0),
    iDaysRemaining(0),
    iTimeNotify(gutil_time_notify_new()),
    iTimeNotifyId(gutil_time_notify_add_handler(iTimeNotify,
        systemTimeChanged, aPass))
{
}

HslCardPeriodPass::Private::~Private()
{
    gutil_time_notify_remove_handler(iTimeNotify, iTimeNotifyId);
    gutil_time_notify_unref(iTimeNotify);
}

void HslCardPeriodPass::Private::setHexData(QString aHexData)
{
    iHexData = aHexData;
    QByteArray hexData(aHexData.toLatin1());
    HDEBUG(hexData.constData());
    GBytes* bytes = gutil_hex2bytes(hexData.constData(), hexData.size());
    iValidityArea = HslArea();
    iPeriodStartDate = QDateTime();
    iPeriodEndDate = QDateTime();
    iLoadingTime = QDateTime();
    iLoadedPeriodDays = 0;
    iLoadedPeriodPrice = 0;
    iDaysRemaining = 0;
    if (bytes) {
        GUtilData data;
        gutil_data_from_bytes(&data, bytes);
        HDEBUG("  ProductCodeType1 =" << getInt(&data, 0, 1));
        HDEBUG("  ProductCode1 =" << getInt(&data, 1, 14));
        const HslArea validityArea1 = getArea(&data, 1, 7, 2, 1);
        HDEBUG("  ValidityAreaType1 =" << getInt(&data, 1, 7, 2));
        HDEBUG("  ValidityArea1 =" << getInt(&data, 2, 1, 6) << validityArea1);
        const QDate periodStartDate1 = getDate(&data, 2, 7);
        HDEBUG("  PeriodStartDate1 =" << getInt(&data, 2, 7, 14) << periodStartDate1);
        const QDate periodEndDate1 = getDate(&data, 4, 5);
        HDEBUG("  PeriodEndDate1 =" << getInt(&data, 4, 5, 14) << periodEndDate1);
        HDEBUG("  ProductCodeType2 =" << getInt(&data, 7, 0, 1));
        HDEBUG("  ProductCode2 =" << getInt(&data, 7, 1, 14));
        const HslArea validityArea2 = getArea(&data, 8, 7, 9, 1);
        HDEBUG("  ValidityAreaType2 =" << getInt(&data,  8, 7, 2));
        HDEBUG("  ValidityArea2 =" << getInt(&data, 9, 1, 6) << validityArea2);
        const QDate periodStartDate2 = getDate(&data, 9, 7);
        HDEBUG("  PeriodStartDate2 =" << getInt(&data, 9, 7, 14) << periodStartDate2);
        const QDate periodEndDate2 = getDate(&data, 11, 5);
        HDEBUG("  PeriodEndDate2 =" << getInt(&data, 11, 5, 14) << periodEndDate2);
        HDEBUG("  ProductCodeType =" << getInt(&data, 14, 0, 1));
        HDEBUG("  ProductCode =" << getInt(&data, 14, 1, 14));
        const QDate loadingDate(getDate(&data, 15, 7));
        const QTime loadingTime(getTime(&data, 17, 5));
        iLoadingTime = QDateTime(loadingDate, loadingTime, Util::FINLAND_TIMEZONE);
        HDEBUG("  LoadingDate =" << getInt(&data, 15, 7, 14) << loadingDate);
        HDEBUG("  LoadingTime =" << getInt(&data, 17, 5, 11) << loadingTime);
        iLoadedPeriodDays = getInt(&data, 19, 0, 9);
        HDEBUG("  LoadedPeriodDays =" << iLoadedPeriodDays);
        iLoadedPeriodPrice = getInt(&data, 20, 1, 20);
        HDEBUG("  PriceOfPeriod =" << iLoadedPeriodPrice);
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
        g_bytes_unref(bytes);

        const bool validPeriod1 = isValidPeriod(periodStartDate1, periodEndDate1);
        const bool validPeriod2 = isValidPeriod(periodStartDate2, periodEndDate2);
        if (validPeriod1) {
            if (!validPeriod2 || periodEndDate1 > periodEndDate2) {
                iPeriodStartDate = startDateTime(periodStartDate1);
                iPeriodEndDate = endDateTime(periodEndDate1);
                iValidityArea = validityArea1;
            } else {
                iPeriodStartDate = startDateTime(periodStartDate2);
                iPeriodEndDate = endDateTime(periodEndDate2);
                iValidityArea = validityArea2;
            }
        } else if (validPeriod2) {
            iPeriodStartDate = startDateTime(periodStartDate2);
            iPeriodEndDate = endDateTime(periodEndDate2);
            iValidityArea = validityArea2;
        }
        updateDaysRemaining();
    }
}

void HslCardPeriodPass::Private::systemTimeChanged(GUtilTimeNotify*, void* aTicket)
{
    HDEBUG("System time changed");
    ((HslCardPeriodPass*)aTicket)->updateDaysRemaining();
}

void HslCardPeriodPass::Private::updateDaysRemaining()
{
    const QDate firstDay(iPeriodStartDate.date());
    const QDate lastDay(iPeriodEndDate.date());
    if (HslData::isValidPeriod(firstDay, lastDay)) {
        const QDateTime now = QDateTime::currentDateTime();
        const QDate today = now.date();
        if (today < firstDay) {
            iDaysRemaining = TravelCard::PeriodNotYetStarted;
        } else if (today > lastDay) {
            iDaysRemaining = TravelCard::PeriodEnded;
        } else {
            const QDateTime nextMidnight(today.addDays(1), QTime(0,0), Util::FINLAND_TIMEZONE);
            iDaysRemaining = today.daysTo(lastDay) + 1;
            HDEBUG(now.secsTo(nextMidnight) << "sec until midnight");
            QTimer::singleShot(now.msecsTo(nextMidnight) + 1000, iPass,
                SLOT(updateDaysRemaining()));
        }
    } else {
        iDaysRemaining = TravelCard::PeriodInvalid;
    }
}

// ==========================================================================
// HslCardPeriodPass
// ==========================================================================

HslCardPeriodPass::HslCardPeriodPass(QObject* aParent) :
    HslData(aParent),
    iPrivate(new Private(this))
{
}

HslCardPeriodPass::~HslCardPeriodPass()
{
    delete iPrivate;
}

QString HslCardPeriodPass::data() const
{
    return iPrivate->iHexData;
}

void HslCardPeriodPass::setData(QString aData)
{
    QString data(aData.toLower());
    if (iPrivate->iHexData != data) {
        const HslArea prevValidityArea(iPrivate->iValidityArea);
        const QDateTime prevPeriodStartDate(iPrivate->iPeriodStartDate);
        const QDateTime prevPeriodEndDate(iPrivate->iPeriodEndDate);
        const QDateTime prevLoadingTime(iPrivate->iLoadingTime);
        const int prevLoadedPeriodDays = iPrivate->iLoadedPeriodDays;
        const int prevLoadedPeriodPrice = iPrivate->iLoadedPeriodPrice;
        const int prevDaysRemaining = iPrivate->iDaysRemaining;
        iPrivate->setHexData(data);
        if (prevValidityArea != iPrivate->iValidityArea) {
            Q_EMIT validityAreaChanged();
        }
        if (prevPeriodStartDate != iPrivate->iPeriodStartDate) {
            Q_EMIT periodStartDateChanged();
        }
        if (prevPeriodEndDate != iPrivate->iPeriodEndDate) {
            Q_EMIT periodEndDateChanged();
        }
        if (prevLoadingTime != iPrivate->iLoadingTime) {
            Q_EMIT loadingTimeChanged();
        }
        if (prevLoadedPeriodDays != iPrivate->iLoadedPeriodDays) {
            Q_EMIT loadedPeriodDaysChanged();
        }
        if (prevLoadedPeriodPrice != iPrivate->iLoadedPeriodPrice) {
            Q_EMIT loadedPeriodPriceChanged();
        }
        if (prevDaysRemaining != iPrivate->iDaysRemaining) {
            Q_EMIT daysRemainingChanged();
        }
        Q_EMIT dataChanged();
    }
}

HslArea HslCardPeriodPass::validityArea() const
{
    return iPrivate->iValidityArea;
}

QString HslCardPeriodPass::validityAreaName() const
{
    return iPrivate->iValidityArea.name();
}

QDateTime HslCardPeriodPass::periodStartDate() const
{
    return iPrivate->iPeriodStartDate;
}

QDateTime HslCardPeriodPass::periodEndDate() const
{
    return iPrivate->iPeriodEndDate;
}

QDateTime HslCardPeriodPass::loadingTime() const
{
    return iPrivate->iLoadingTime;
}

int HslCardPeriodPass::loadedPeriodDays() const
{
    return iPrivate->iLoadedPeriodDays;
}

int HslCardPeriodPass::loadedPeriodPrice() const
{
    return iPrivate->iLoadedPeriodPrice;
}

int HslCardPeriodPass::daysRemaining() const
{
    return iPrivate->iDaysRemaining;
}

void HslCardPeriodPass::updateDaysRemaining()
{
    const int prevDaysRemaining = iPrivate->iDaysRemaining;
    iPrivate->updateDaysRemaining();
    if (prevDaysRemaining != iPrivate->iDaysRemaining) {
        Q_EMIT daysRemainingChanged();
    }
}

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

#include "HslCardEticket.h"

#include <gutil_misc.h>
#include <gutil_timenotify.h>

#include "HarbourDebug.h"

// ==========================================================================
// HslCardEticket::Private
// ==========================================================================

class HslCardEticket::Private {
public:
    Private(HslCardEticket* aTicket);
    ~Private();

    void setHexData(QString aHexData);
    void updateSecondsRemaining();

    static void systemTimeChanged(GUtilTimeNotify*, void*);

public:
    HslCardEticket* iTicket;
    QString iHexData;
    Language iLanguage;
    ValidityLengthType iValidityLengthType;
    int iValidityLength;
    HslArea iValidityArea;
    int iTicketPrice;
    int iGroupSize;
    bool iExtraZone;
    int iExtensionFare;
    QDateTime iValidityStartTime;
    QDateTime iValidityEndTime;
    QDateTime iValidityEndTimeGroup;
    QDateTime iBoardingTime;
    int iBoardingVehicle;
    HslArea iBoardingArea;
    int iSecondsRemaining;
    GUtilTimeNotify* iTimeNotify;
    gulong iTimeNotifyId;
};

HslCardEticket::Private::Private(HslCardEticket* aTicket) :
    iTicket(aTicket),
    iLanguage(LanguageUnknown),
    iValidityLengthType(ValidityLengthUnknown),
    iValidityLength(0),
    iTicketPrice(0),
    iGroupSize(0),
    iExtraZone(false),
    iExtensionFare(0),
    iBoardingVehicle(0),
    iSecondsRemaining(0),
    iTimeNotify(gutil_time_notify_new()),
    iTimeNotifyId(gutil_time_notify_add_handler(iTimeNotify,
        systemTimeChanged, aTicket))
{
}

HslCardEticket::Private::~Private()
{
    gutil_time_notify_remove_handler(iTimeNotify, iTimeNotifyId);
    gutil_time_notify_unref(iTimeNotify);
}

void HslCardEticket::Private::setHexData(QString aHexData)
{
    iHexData = aHexData;
    QByteArray hexData(aHexData.toLatin1());
    HDEBUG(hexData.constData());
    GBytes* bytes = gutil_hex2bytes(hexData.constData(), hexData.size());
    iLanguage = LanguageUnknown;
    iValidityLengthType = ValidityLengthUnknown;
    iValidityLength = 0;
    iValidityArea = HslArea();
    iTicketPrice = 0;
    iGroupSize = 0;
    iExtraZone = false;
    iExtensionFare = 0;
    iValidityStartTime = QDateTime();
    iValidityEndTime = QDateTime();
    iValidityEndTimeGroup = QDateTime();
    iBoardingTime = QDateTime();
    iBoardingVehicle = 0;
    iBoardingArea = HslArea();
    if (bytes) {
        GUtilData data;
        gutil_data_from_bytes(&data, bytes);
        HDEBUG("  ProductCodeType =" << getInt(&data, 0, 1));
        HDEBUG("  ProductCode =" << getInt(&data, 0, 14));
        HDEBUG("  ProductCodeGroup =" << getInt(&data, 1, 7, 14));
        HDEBUG("  CustomerProfile =" << getInt(&data, 3, 5, 5));
        HDEBUG("  CustomerProfileGroup =" << getInt(&data, 4, 2, 5));
        int languageCode = getInt(&data, 4, 7, 2);
        HDEBUG("  LanguageCode =" << languageCode);
        int validityLengthType = getInt(&data, 5, 1, 2);
        HDEBUG("  ValidityLengthType =" << validityLengthType);
        iValidityLength = getInt(&data, 5, 3, 8);
        HDEBUG("  ValidityLength =" << iValidityLength);
        HDEBUG("  ValidityLengthTypeGroup =" << getInt(&data, 6, 3, 2));
        HDEBUG("  ValidityLengthGroup =" << getInt(&data, 6, 5, 8));
        iValidityArea = getArea(&data, 7, 5, 7, 7);
        HDEBUG("  ValidityAreaType =" << getInt(&data, 7, 5, 2));
        HDEBUG("  ValidityArea =" << getInt(&data, 7, 7, 6) << iValidityArea);
        HDEBUG("  SaleDate =" << getInt(&data, 8, 5, DATE_BITS) << getDate(&data, 8, 5));
        HDEBUG("  SaleTime =" << getInt(&data, 10, 3, 5) <<
            START_TIME.addSecs(getInt(&data, 10, 3, 5) * 60));
        HDEBUG("  SaleDeviceType =" << getInt(&data, 11, 0, 3));
        HDEBUG("  SaleDeviceNumber =" << getInt(&data, 11, 3, 14));
        iTicketPrice = getInt(&data, 13, 1, 14);
        HDEBUG("  TicketFare =" << iTicketPrice);
        const int groupFare = getInt(&data, 14, 7, 14);
        HDEBUG("  TicketFareGroup =" << groupFare);
        iGroupSize = getInt(&data, 16, 5, 6);
        if (iGroupSize > 1) {
            iTicketPrice += groupFare;
        }
        HDEBUG("  GroupSize =" << iGroupSize);
        iExtraZone = (getInt(&data, 17, 3, 1) != 0);
        HDEBUG("  ExtraZone =" << iExtraZone);
        HDEBUG("  PeriodPassValidityArea =" << getInt(&data, 17, 4, 6));
        HDEBUG("  ExtensionProductCode =" << getInt(&data, 18, 2, 14));
        HDEBUG("  Extension1ValidityArea =" << getInt(&data, 20, 0, 6));
        iExtensionFare = getInt(&data, 20, 6, 14);
        HDEBUG("  Extension1Fare =" << iExtensionFare);
        HDEBUG("  Extension2ValidityArea =" << getInt(&data, 22, 4, 6));
        HDEBUG("  Extension2Fare =" << getInt(&data, 23, 2, 14));
        HDEBUG("  SaleStatus =" << getInt(&data, 25, 0, 1));
        const QDate validityStartDate(getDate(&data, 25, 5));
        const QTime validityStartTime(getTime(&data, 27, 3));
        iValidityStartTime = QDateTime(validityStartDate, validityStartTime, HELSINKI_TIMEZONE);
        HDEBUG("  ValidityStartDate =" << getInt(&data, 25, 5, DATE_BITS) << validityStartDate);
        HDEBUG("  ValidityStartTime =" << getInt(&data, 27, 3, TIME_BITS) << validityStartTime);
        const QDate validityEndDate(getDate(&data, 28, 6));
        const QTime validityEndTime(getTime(&data, 30, 4));
        iValidityEndTime = QDateTime(validityEndDate, validityEndTime, HELSINKI_TIMEZONE);
        HDEBUG("  ValidityEndDate =" << getInt(&data, 28, 6, DATE_BITS) << validityEndDate);
        HDEBUG("  ValidityEndTime =" << getInt(&data, 30, 4, TIME_BITS) << validityEndTime);
        const QDate validityEndDateGroup(getDate(&data, 31, 7));
        const QTime validityEndTimeGroup(getTime(&data, 33, 5));
        iValidityEndTimeGroup = QDateTime(validityEndDateGroup, validityEndTimeGroup, HELSINKI_TIMEZONE);
        HDEBUG("  ValidityEndDateGroup =" << getInt(&data, 31, 7, DATE_BITS) << validityEndDateGroup);
        HDEBUG("  ValidityEndTimeGroup =" << getInt(&data, 33, 5, TIME_BITS) << validityEndTimeGroup);
        HDEBUG("  ValidityStatus =" << getInt(&data, 35, 5, 1));
        const QDate boardingDate(getDate(&data, 35, 6));
        const QTime boardingTime(getTime(&data, 37, 4));
        iBoardingTime = QDateTime(boardingDate, boardingTime, HELSINKI_TIMEZONE);
        HDEBUG("  BoardingDate =" << getInt(&data, 35, 6, DATE_BITS) << boardingDate);
        HDEBUG("  BoardingTime =" << getInt(&data, 37, 4, TIME_BITS) << boardingTime);
        iBoardingVehicle = getInt(&data, 38, 7, 14);
        HDEBUG("  BoardingVehicle =" << iBoardingVehicle);
        HDEBUG("  BoardingLocationNumberType =" << getInt(&data, 40, 5, 2));
        HDEBUG("  BoardingLocationNumber =" << getInt(&data, 40, 7, 14));
        HDEBUG("  BoardingDirection =" << getInt(&data, 42, 5, 1));
        iBoardingArea = getArea(&data, 42, 6, 43, 0);
        HDEBUG("  BoardingAreaType =" << getInt(&data, 42, 6, 2));
        HDEBUG("  BoardingArea =" << getInt(&data, 43, 0, 6) << iBoardingArea);
        g_bytes_unref(bytes);

        // Kielikoodi: 0=Suomi, 1=Ruotsi, 2=Englanti
        switch (languageCode) {
        case 0: iLanguage = LanguageFinnish; break;
        case 1: iLanguage = LanguageSwedish; break;
        case 2: iLanguage = LanguageEnglish; break;
        }

        // Voimassaolon pituuden tyyppi:
        // 0=Minuutteja, 1=Tunteja,
        // 2=Vuorokausia, 3=Päiviä
        switch (validityLengthType) {
        case 0: iValidityLengthType = ValidityLengthMinute; break;
        case 1: iValidityLengthType = ValidityLengthHour; break;
        case 2: iValidityLengthType = ValidityLength24Hours; break;
        case 3: iValidityLengthType = ValidityLengthDay; break;
        }
        updateSecondsRemaining();
    }
}

void HslCardEticket::Private::systemTimeChanged(GUtilTimeNotify*, void* aTicket)
{
    HDEBUG("System time changed");
    ((HslCardEticket*)aTicket)->updateSecondsRemaining();
}

void HslCardEticket::Private::updateSecondsRemaining()
{
    if (HslData::isValidTimePeriod(iValidityStartTime, iValidityEndTime)) {
        const QDateTime now = QDateTime::currentDateTime();
        if (now < iValidityStartTime) {
            iSecondsRemaining = PeriodNotYetStarted;
        } else if (now > iValidityEndTime) {
            iSecondsRemaining = PeriodEnded;
        } else {
            qint64 msecsRemaining = now.msecsTo(iValidityEndTime);
            iSecondsRemaining = (int)(msecsRemaining/1000) + 1;
            int nextInterval = (msecsRemaining % 1000);
            if (!nextInterval) nextInterval = 1000;
            HDEBUG(nextInterval << "ms until next update");
            QTimer::singleShot(nextInterval, iTicket, SLOT(updateSecondsRemaining()));
        }
    } else {
        iSecondsRemaining = PeriodInvalid;
    }
}

// ==========================================================================
// HslCardEticket
// ==========================================================================

HslCardEticket::HslCardEticket(QObject* aParent) :
    HslData(aParent),
    iPrivate(new Private(this))
{
}

HslCardEticket::~HslCardEticket()
{
    delete iPrivate;
}

QString HslCardEticket::data() const
{
    return iPrivate->iHexData;
}

void HslCardEticket::setData(QString aData)
{
    QString data(aData.toLower());
    if (iPrivate->iHexData != data) {
        const Language prevLanguage = iPrivate->iLanguage;
        const ValidityLengthType prevValidityLengthType = iPrivate->iValidityLengthType;
        const int prevValidityLength = iPrivate->iValidityLength;
        const HslArea prevValidityArea = iPrivate->iValidityArea;
        const int prevTicketPrice = iPrivate->iTicketPrice;
        const int prevGroupSize = iPrivate->iGroupSize;
        const bool prevExtraZone = iPrivate->iExtraZone;
        const int prevExtensionFare = iPrivate->iExtensionFare;
        const QDateTime prevValidityStartTime(iPrivate->iValidityStartTime);
        const QDateTime prevValidityEndTime(iPrivate->iValidityEndTime);
        const QDateTime prevValidityEndTimeGroup(iPrivate->iValidityEndTimeGroup);
        const QDateTime prevBoardingTime(iPrivate->iBoardingTime);
        const int prevBoardingVehicle = iPrivate->iBoardingVehicle;
        const HslArea prevBoardingArea = iPrivate->iBoardingArea;
        const int prevSecondsRemaining = iPrivate->iSecondsRemaining;
        iPrivate->setHexData(data);
        if (prevLanguage != iPrivate->iLanguage) {
            Q_EMIT languageChanged();
        }
        if (prevValidityLengthType != iPrivate->iValidityLengthType) {
            Q_EMIT validityLengthTypeChanged();
        }
        if (prevValidityLength != iPrivate->iValidityLength) {
            Q_EMIT validityLengthChanged();
        }
        if (prevValidityArea != iPrivate->iValidityArea) {
            Q_EMIT validityAreaChanged();
        }
        if (prevTicketPrice != iPrivate->iTicketPrice) {
            Q_EMIT ticketPriceChanged();
        }
        if (prevGroupSize != iPrivate->iGroupSize) {
            Q_EMIT groupSizeChanged();
        }
        if (prevExtraZone != iPrivate->iExtraZone) {
            Q_EMIT extraZoneChanged();
        }
        if (prevExtensionFare != iPrivate->iExtensionFare) {
            Q_EMIT extensionFareChanged();
        }
        if (prevValidityStartTime != iPrivate->iValidityStartTime) {
            Q_EMIT validityStartTimeChanged();
        }
        if (prevValidityEndTime != iPrivate->iValidityEndTime) {
            Q_EMIT validityEndTimeChanged();
        }
        if (prevValidityEndTimeGroup != iPrivate->iValidityEndTimeGroup) {
            Q_EMIT validityEndTimeGroupChanged();
        }
        if (prevBoardingTime != iPrivate->iBoardingTime) {
            Q_EMIT boardingTimeChanged();
        }
        if (prevBoardingVehicle != iPrivate->iBoardingVehicle) {
            Q_EMIT boardingVehicleChanged();
        }
        if (prevBoardingArea != iPrivate->iBoardingArea) {
            Q_EMIT boardingAreaChanged();
        }
        if (prevSecondsRemaining != iPrivate->iSecondsRemaining) {
            Q_EMIT secondsRemainingChanged();
        }
        Q_EMIT dataChanged();
    }
}

HslData::Language HslCardEticket::language() const
{
    return iPrivate->iLanguage;
}

HslData::ValidityLengthType HslCardEticket::validityLengthType() const
{
    return iPrivate->iValidityLengthType;
}

int HslCardEticket::validityLength() const
{
    return iPrivate->iValidityLength;
}

HslArea HslCardEticket::validityArea() const
{
    return iPrivate->iValidityArea;
}

QString HslCardEticket::validityAreaName() const
{
    return iPrivate->iValidityArea.name();
}

int HslCardEticket::ticketPrice() const
{
    return iPrivate->iTicketPrice;
}

int HslCardEticket::groupSize() const
{
    return iPrivate->iGroupSize;
}

bool HslCardEticket::extraZone() const
{
    return iPrivate->iExtraZone;
}

int HslCardEticket::extensionFare() const
{
    return iPrivate->iExtensionFare;
}

QDateTime HslCardEticket::validityStartTime() const
{
    return iPrivate->iValidityStartTime;
}

QDateTime HslCardEticket::validityEndTime() const
{
    return iPrivate->iValidityEndTime;
}

QDateTime HslCardEticket::validityEndTimeGroup() const
{
    return iPrivate->iValidityEndTimeGroup;
}

QDateTime HslCardEticket::boardingTime() const
{
    return iPrivate->iBoardingTime;
}

int HslCardEticket::boardingVehicle() const
{
    return iPrivate->iBoardingVehicle;
}

HslArea HslCardEticket::boardingArea() const
{
    return iPrivate->iBoardingArea;
}

QString HslCardEticket::boardingAreaName() const
{
    return iPrivate->iBoardingArea.name();
}

int HslCardEticket::secondsRemaining() const
{
    return iPrivate->iSecondsRemaining;
}

void HslCardEticket::updateSecondsRemaining()
{
    const int prevSecondsRemaining = iPrivate->iSecondsRemaining;
    iPrivate->updateSecondsRemaining();
    if (prevSecondsRemaining != iPrivate->iSecondsRemaining) {
        Q_EMIT secondsRemainingChanged();
    }
}

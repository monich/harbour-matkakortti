/*
 * Copyright (C) 2020 Jolla Ltd.
 * Copyright (C) 2020 Slava Monich <slava.monich@jolla.com>
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

#include "gutil_misc.h"
#include "gutil_timenotify.h"

#include "NysseCardSeasonPass.h"
#include "NysseUtil.h"
#include "TravelCard.h"
#include "Util.h"

#include "HarbourDebug.h"

// ==========================================================================
// NysseCardSeasonPass::Private
// ==========================================================================

class NysseCardSeasonPass::Private {
public:
    Private(NysseCardSeasonPass* aPass);
    ~Private();

    void setHexData(QString aHexData);
    void updateDaysRemaining();

    static void systemTimeChanged(GUtilTimeNotify*, void*);

public:
    NysseCardSeasonPass* iPass;
    QString iHexData;
    QDateTime iEndDate;
    bool iValid;
    int iDaysRemaining;
    GUtilTimeNotify* iTimeNotify;
    gulong iTimeNotifyId;
};

NysseCardSeasonPass::Private::Private(NysseCardSeasonPass* aPass) :
    iPass(aPass),
    iValid(false),
    iDaysRemaining(0),
    iTimeNotify(gutil_time_notify_new()),
    iTimeNotifyId(gutil_time_notify_add_handler(iTimeNotify,
        systemTimeChanged, aPass))
{
}

NysseCardSeasonPass::Private::~Private()
{
    gutil_time_notify_remove_handler(iTimeNotify, iTimeNotifyId);
    gutil_time_notify_unref(iTimeNotify);
}

void NysseCardSeasonPass::Private::setHexData(QString aHexData)
{
    iHexData = aHexData;
    HDEBUG(qPrintable(iHexData));
    iEndDate = QDateTime();
    iValid = false;
    iDaysRemaining = TravelCard::PeriodInvalid;
    guint8 data[16];
    QByteArray hex(iHexData.toLatin1());
    if (gutil_hex2bin(hex.constData(), 2*sizeof(data), data)) {
        iValid = (data[6] != 0);
        HDEBUG("  Valid =" << iValid);
        if (iValid) {
            iEndDate = NysseUtil::toDateTime(Util::uint16be(data + 10), 0);
            HDEBUG("  EndDate =" << iEndDate);
            updateDaysRemaining();
        }
    }
}

void NysseCardSeasonPass::Private::systemTimeChanged(GUtilTimeNotify*, void* aPass)
{
    HDEBUG("System time changed");
    ((NysseCardSeasonPass*)aPass)->updateDaysRemaining();
}

void NysseCardSeasonPass::Private::updateDaysRemaining()
{
    if (iValid) {
        const QDateTime now = QDateTime::currentDateTime();
        const QDate today = now.date();
        const QDate lastDay(iEndDate.date());
        if (today > lastDay) {
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
// NysseCardSeasonPass
// ==========================================================================

NysseCardSeasonPass::NysseCardSeasonPass(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

NysseCardSeasonPass::~NysseCardSeasonPass()
{
    delete iPrivate;
}

QString NysseCardSeasonPass::data() const
{
    return iPrivate->iHexData;
}

void NysseCardSeasonPass::setData(QString aData)
{
    QString data(aData.toLower());
    if (iPrivate->iHexData != data) {
        const QDateTime prevEndDate(iPrivate->iEndDate);
        const bool prevValid(iPrivate->iValid);
        const int prevDaysRemaining = iPrivate->iDaysRemaining;
        iPrivate->setHexData(data);
        if (prevEndDate != iPrivate->iEndDate) {
            Q_EMIT endDateChanged();
        }
        if (prevValid != iPrivate->iValid) {
            Q_EMIT validChanged();
        }
        if (prevDaysRemaining != iPrivate->iDaysRemaining) {
            Q_EMIT daysRemainingChanged();
        }
        Q_EMIT dataChanged();
    }
}

bool NysseCardSeasonPass::valid() const
{
    return iPrivate->iValid;
}

int NysseCardSeasonPass::daysRemaining() const
{
    return iPrivate->iDaysRemaining;
}

QDateTime NysseCardSeasonPass::endDate() const
{
    return iPrivate->iEndDate;
}

void NysseCardSeasonPass::updateDaysRemaining()
{
    const int prevDaysRemaining = iPrivate->iDaysRemaining;
    iPrivate->updateDaysRemaining();
    if (prevDaysRemaining != iPrivate->iDaysRemaining) {
        Q_EMIT daysRemainingChanged();
    }
}

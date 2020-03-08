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

#include "HslCardStoredValue.h"
#include "Util.h"

#include <gutil_misc.h>

#include "HarbourDebug.h"

// ==========================================================================
// HslCardStoredValue::Private
// ==========================================================================

class HslCardStoredValue::Private {
public:
    Private();

    void setHexData(QString aHexData);

public:
    QString iHexData;
    int iMoneyValue;
    QDateTime iLoadingTime;
    int iLoadedValue;
};

HslCardStoredValue::Private::Private() :
    iMoneyValue(0),
    iLoadedValue(0)
{
}

void HslCardStoredValue::Private::setHexData(QString aHexData)
{
    iHexData = aHexData;
    QByteArray hexData(aHexData.toLatin1());
    HDEBUG(hexData.constData());
    GBytes* bytes = gutil_hex2bytes(hexData.constData(), hexData.size());
    iMoneyValue = 0;
    iLoadingTime = QDateTime();
    iLoadedValue = 0;
    if (bytes) {
        GUtilData data;
        gutil_data_from_bytes(&data, bytes);
        iMoneyValue = getInt(&data, 0, 20);
        HDEBUG("  ValueCounter =" << iMoneyValue);
        const QDate loadingDate(getDate(&data, 2, 4));
        const QTime loadingTime(getTime(&data, 4, 2));
        iLoadingTime = QDateTime(loadingDate, loadingTime, Util::FINLAND_TIMEZONE);
        HDEBUG("  LoadingDate =" << getInt(&data, 2, 4, 14) << loadingDate);
        HDEBUG("  LoadingTime =" << getInt(&data, 4, 2, 11) << loadingTime);
        iLoadedValue = getInt(&data, 5, 5, 20);
        HDEBUG("  LoadedValue =" << iLoadedValue);
        HDEBUG("  LoadingOrganisationID =" << getInt(&data, 8, 1, 14));
        HDEBUG("  LoadingDeviceNumber =" << getInt(&data, 9, 7, 14));
        g_bytes_unref(bytes);
    }
}

// ==========================================================================
// HslCardStoredValue
// ==========================================================================

HslCardStoredValue::HslCardStoredValue(QObject* aParent) :
    HslData(aParent),
    iPrivate(new Private)
{
}

HslCardStoredValue::~HslCardStoredValue()
{
    delete iPrivate;
}

QString HslCardStoredValue::data() const
{
    return iPrivate->iHexData;
}

void HslCardStoredValue::setData(QString aData)
{
    QString data(aData.toLower());
    if (iPrivate->iHexData != data) {
        const int prevMoneyValue = iPrivate->iMoneyValue;
        const QDateTime prevLoadingTime(iPrivate->iLoadingTime);
        const int prevLoadedValue = iPrivate->iLoadedValue;
        iPrivate->setHexData(data);
        if (prevMoneyValue != iPrivate->iMoneyValue) {
            Q_EMIT moneyValueChanged();
        }
        if (prevLoadingTime != iPrivate->iLoadingTime) {
            Q_EMIT loadingTimeChanged();
        }
        if (prevLoadedValue != iPrivate->iLoadedValue) {
            Q_EMIT loadedValueChanged();
        }
        Q_EMIT dataChanged();
    }
}

int HslCardStoredValue::moneyValue() const
{
    return iPrivate->iMoneyValue;
}

int HslCardStoredValue::loadedValue() const
{
    return iPrivate->iLoadedValue;
}

QDateTime HslCardStoredValue::loadingTime() const
{
    return iPrivate->iLoadingTime;
}

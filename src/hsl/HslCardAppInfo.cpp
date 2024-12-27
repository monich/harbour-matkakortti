/*
 * Copyright (C) 2019-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2019 Jolla Ltd.
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

#include "HslCardAppInfo.h"
#include "HslData.h"

#include "HarbourDebug.h"
#include "Util.h"

// ==========================================================================
// HslCardAppInfo::Private
// ==========================================================================

class HslCardAppInfo::Private
{
public:
    Private();

    void setHexData(const QString&);

public:
    int iAppVersion;
    QString iHexData;
    QString iCardNumber;
};

HslCardAppInfo::Private::Private() :
    iAppVersion(0)
{}

void
HslCardAppInfo::Private::setHexData(
    const QString& aHexData)
{
    QByteArray hexData(aHexData.toLatin1());
    const QByteArray bytes(QByteArray::fromHex(hexData));

    HDEBUG(hexData.constData());
    iHexData = aHexData;
    iCardNumber.clear();
    iAppVersion = 0;

    if (!bytes.isEmpty()) {
        const GUtilData data = Util::toData(bytes);

        // ApplicationInformation
        //
        // +======================================================+
        // | Byte/Bit | Bit   | Entry | Description               |
        // | offset   | count | type  |                           |
        // +==========+=======+=======+===========================+
        // | 0/0      | 4     | uint  | Application version       |
        // | 0/4      | 4     | -     | Reserved                  |
        // | 1/0      | 72    | bcd   | Card number               |
        // | 10/0     | 3     | uint  | Platform type             |
        // | 10/3     | 1     | uint  | Security level            |
        // +======================================================+
        iAppVersion = HslData::getInt(&data, 0, 4);
        HDEBUG("  ApplicationVersion =" << iAppVersion);
        iCardNumber = iHexData.mid(2, 18);
        HDEBUG("  CardNumber =" << iCardNumber);
        HDEBUG("  PlatformType =" << HslData::getInt(&data, 10, 0, 3));
        HDEBUG("  SecurityLevel =" << HslData::getInt(&data, 10, 3, 1));
    }
}

// ==========================================================================
// HslCardAppInfo
// ==========================================================================

HslCardAppInfo::HslCardAppInfo(
    QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private)
{
}

HslCardAppInfo::~HslCardAppInfo()
{
    delete iPrivate;
}

QString
HslCardAppInfo::data() const
{
    return iPrivate->iHexData;
}

void
HslCardAppInfo::setData(
    QString aData)
{
    const QString data(aData.toLower());
    if (iPrivate->iHexData != data) {
        const int appVersion(iPrivate->iAppVersion);
        const QString cardNumber(iPrivate->iCardNumber);

        iPrivate->setHexData(data);
        if (appVersion != iPrivate->iAppVersion) {
            Q_EMIT appVersionChanged();
        }
        if (cardNumber != iPrivate->iCardNumber) {
            Q_EMIT cardNumberChanged();
        }
        Q_EMIT dataChanged();
    }
}

int
HslCardAppInfo::appVersion() const
{
    return iPrivate->iAppVersion;
}

QString
HslCardAppInfo::cardNumber() const
{
    return iPrivate->iCardNumber;
}

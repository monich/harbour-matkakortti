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

#include "NysseCardBalance.h"

#include "HarbourDebug.h"

// ==========================================================================
// NysseCardBalance::Private
// ==========================================================================

class NysseCardBalance::Private {
public:
    Private() : iBalance(0) {}

    void setHexData(QString aHexData);

public:
    QString iHexData;
    uint iBalance;
};

void NysseCardBalance::Private::setHexData(QString aHexData)
{
    iHexData = aHexData;
    HDEBUG(qPrintable(iHexData));
    guint32 data;
    QByteArray hex(iHexData.toLatin1());
    if (hex.size() == sizeof(data) * 2 &&
        gutil_hex2bin(hex.constData(), hex.size(), &data)) {
        iBalance = GINT32_FROM_LE(data);
        HDEBUG("  Balance =" << iBalance);
    } else {
        iBalance = 0;
    }
}

// ==========================================================================
// NysseCardBalance
// ==========================================================================

NysseCardBalance::NysseCardBalance(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private)
{
}

NysseCardBalance::~NysseCardBalance()
{
    delete iPrivate;
}

QString NysseCardBalance::data() const
{
    return iPrivate->iHexData;
}

void NysseCardBalance::setData(QString aData)
{
    QString data(aData.toLower());
    if (iPrivate->iHexData != data) {
        const uint prevBalance(iPrivate->iBalance);
        iPrivate->setHexData(data);
        if (prevBalance != iPrivate->iBalance) {
            Q_EMIT balanceChanged();
        }
        Q_EMIT dataChanged();
    }
}

uint NysseCardBalance::balance() const
{
    return iPrivate->iBalance;
}

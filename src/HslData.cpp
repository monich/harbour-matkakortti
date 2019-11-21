/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019 Slava Monich <slava@monich.com>
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

#include "HslData.h"

#include "HarbourDebug.h"

const QDate HslData::START_DATE(1997, 1, 1);
const QTime HslData::START_TIME(0, 0);
const QTimeZone HslData::HELSINKI_TIMEZONE("Europe/Helsinki");

HslData::HslData(QObject* aParent) :
    QObject(aParent)
{
}

QObject* HslData::createSingleton(QQmlEngine*, QJSEngine*)
{
    return new HslData;
}

bool HslData::isValidDate(QDate aDate)
{
    return aDate > START_DATE;
}

bool HslData::isValidPeriod(QDate aStart, QDate aEnd)
{
    return isValidDate(aStart) && isValidDate(aEnd) && aStart <= aEnd;
}

bool HslData::isValidTimePeriod(QDateTime aStart, QDateTime aEnd)
{
    return isValidDate(aStart.date()) && isValidDate(aEnd.date()) && aStart <= aEnd;
}

QDateTime HslData::startDateTime(QDate aDate)
{
    return QDateTime(aDate, QTime(0,0), HELSINKI_TIMEZONE);
}

QDateTime HslData::endDateTime(QDate aDate)
{
    return QDateTime(aDate, QTime(23,59,59,999), HELSINKI_TIMEZONE);
}

uint HslData::getInt(const GUtilData* aData, uint aByteOffset,
    uint aBitOffset, uint aCount)
{
    return getInt(aData, 8 * aByteOffset + aBitOffset, aCount);
}

uint HslData::getInt(const GUtilData* aData, uint aOffset, uint aCount)
{
    if (aData) {
        const gsize totalBits = aData->size * 8;
        if (aOffset < totalBits) {
            uint out = 0;
            const uchar* data = aData->bytes + (aOffset/8);
            HASSERT((aOffset + aCount) <= totalBits);
            if ((aOffset + aCount) > totalBits) {
                aCount = totalBits - aOffset;
            }
            if (aOffset & 7) {
                // Chunk of the first byte
                while (aCount > 0 && (aOffset & 7)) {
                    out = (out << 1) | (((*data) >> (7 - (aOffset & 7))) & 1);
                    aOffset++;
                    aCount--;
                }
                data++;
            }
            // Consume entire bytes
            while (aCount >= 8) {
                out = (out << 8) | (*data++);
                aOffset += 8;
                aCount -= 8;
            }
            // Chunk of the last byte (if any)
            while (aCount > 0) {
                out = (out << 1) | (((*data) >> (7 - (aOffset & 7))) & 1);
                aOffset++;
                aCount--;
            }
            return out;
        }
    }
    return 0;
}

// EN 1545-1, DateStamp (number of days since 1.1.1997)
// DateStamp ::= BIT STRING(SIZE (14))
QDate HslData::getDate(const GUtilData* aData, uint aByteOffset, uint aBitOffset)
{
    return getDate(aData, 8 * aByteOffset + aBitOffset);
}

QDate HslData::getDate(const GUtilData* aData, uint aOffset)
{
    return START_DATE.addDays(getInt(aData, aOffset, DATE_BITS));
}

// EN 1545-1, TimeStamp (number of minutes since 00:00)
// TimeStamp ::= BIT STRING (SIZE(11))
QTime HslData::getTime(const GUtilData* aData, uint aByteOffset, uint aBitOffset)
{
    return getTime(aData, 8 * aByteOffset + aBitOffset);
}

QTime HslData::getTime(const GUtilData* aData, uint aOffset)
{
    return START_TIME.addSecs(getInt(aData, aOffset, TIME_BITS) * 60);
}

HslArea::Type HslData::getAreaType(const GUtilData* aData, uint aByteOffset, uint aBitOffset)
{
    return getAreaType(aData, 8 * aByteOffset + aBitOffset);
}

HslArea::Type HslData::getAreaType(const GUtilData* aData, uint aOffset)
{
    // 0=Vyöhyke, 1=Ajoneuvo, 2=Uusi vyöhyke
    switch (getInt(aData, aOffset, 2)) {
    case 0: return HslArea::Zone;
    case 1: return HslArea::Vehicle;
    case 2: return HslArea::MultiZone;
    default: return HslArea::UnknownArea;
    }
}

HslArea HslData::getArea(const GUtilData* aData, uint aTypeByteOffset, uint aTypeBitOffset,
    uint aAreaByteOffset, uint aAreaBitOffset)
{
    return getArea(aData,
        8 * aTypeByteOffset + aTypeBitOffset,
        8 * aAreaByteOffset + aAreaBitOffset);
}

HslArea HslData::getArea(const GUtilData* aData, uint aTypeOffset, uint aAreaOffset)
{
    HslArea::Type type = HslData::getAreaType(aData, aTypeOffset);
    if (type != HslArea::UnknownArea) {
        return HslArea(type, getInt(aData, aAreaOffset, 6));
    } else {
        return HslArea();
    }
}

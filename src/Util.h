/*
 * Copyright (C) 2019-2022 Jolla Ltd.
 * Copyright (C) 2019-2023 Slava Monich <slava@monich.com>
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

#ifndef UTIL_H
#define UTIL_H

#include "gutil_types.h"

#include <QString>
#include <QByteArray>
#include <QDateTime>
#include <QTimeZone>

namespace Util {
    extern const QString CARD_TYPE_KEY;
    extern const QTimeZone FINLAND_TIMEZONE; // Europe/Helsinki

    guint32 uint32le(const guint8* data);
    guint32 uint32be(const guint8* data);
    guint16 uint16le(const guint8* data);
    guint16 uint16be(const guint8* data);

    inline QByteArray toByteArray(const GUtilData* aData)
        { return QByteArray((const char*)aData->bytes, (int)aData->size); }
    inline QDateTime finnishTime(const QDateTime aDateTime)
        { return aDateTime.toTimeZone(FINLAND_TIMEZONE); }
    inline QDateTime currentTimeInFinland()
        { return finnishTime(QDateTime::currentDateTimeUtc()); }
}

#endif // UTIL_H

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

#ifndef HSL_AREA_H
#define HSL_AREA_H

#include <QMetaType>
#include <QDebug>

class HslArea {
    Q_GADGET
    Q_ENUMS(Type)
    Q_PROPERTY(bool valid READ valid CONSTANT)
    Q_PROPERTY(Type type READ type CONSTANT)
    Q_PROPERTY(int code READ code CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    enum Type {
        UnknownArea,
        Zone,
        Vehicle,
        MultiZone
    };

    HslArea(Type aType, int aCode);
    HslArea(const HslArea& aArea);
    HslArea();
    ~HslArea();

    HslArea& operator = (const HslArea& aArea);
    bool operator == (const HslArea& aArea) const;
    bool operator != (const HslArea& aArea) const;
    bool equals(const HslArea& aArea) const;

    bool valid() const;
    Type type() const;
    int code() const;
    const QString name() const;

private:
    class Private;
    Private* iPrivate;
};

// Debug output
QDebug operator<<(QDebug aDebug, const HslArea& aArea);

// Inline methods
inline bool HslArea::operator == (const HslArea& aArea) const
    { return equals(aArea); }
inline bool HslArea::operator != (const HslArea& aArea) const
    { return !equals(aArea); }

Q_DECLARE_METATYPE(HslArea)

#endif // HSL_AREA_H

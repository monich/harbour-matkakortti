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

#include "HslArea.h"

#include <QAtomicInt>

// ==========================================================================
// HslArea::Private
// ==========================================================================

class HslArea::Private
{
public:
    Private(Type aType, int aCode);
    ~Private();

    static const QString getZoneName(int aCode);
    static const QString getMultiZoneName(int aCode);

public:
    QAtomicInt iRef;
    Type iType;
    int iCode;
    QString iName;
};

HslArea::Private::Private(Type aType, int aCode) :
    iRef(1),
    iType(aType),
    iCode(aCode)
{
    switch (aType) {
    case HslArea::Zone:
        iName = getZoneName(aCode);
        break;
    case HslArea::MultiZone:
        iName = getMultiZoneName(aCode);
        break;
    default:
        break;
    }
}

HslArea::Private::~Private()
{
}

const QString HslArea::Private::getZoneName(int aCode)
{
    switch (aCode) {
    case 1: return QLatin1String("Helsinki");
    case 2: return QLatin1String("Espoo");
    case 4: return QLatin1String("Vantaa");
    case 5: return QLatin1String("Seutu");
    case 6: return QLatin1String("Kirkkonummi-Siuntio");
    case 7: return QLatin1String("Vihti");
    case 8: return QLatin1String("Nurmijärvi");
    case 9: return QLatin1String("Kerava-Sipoo-Tuusula");
    case 10: return QLatin1String("Sipoo");
    case 14: return QLatin1String("Lähiseutu 2");
    case 15: return QLatin1String("Lähiseutu 3");
    default: return QString();
    }
}

const QString HslArea::Private::getMultiZoneName(int aCode)
{
    switch (aCode) {
    case 0: /* no break */
    case 1: return QLatin1String("AB");
    case 2: /* no break */
    case 4: return QLatin1String("BC");
    case 5: return QLatin1String("ABC");
    case 6: /* no break */
    case 9: return QLatin1String("D");
    case 14: return QLatin1String("BCD");
    case 15: return QLatin1String("ABCD");
    default: return QString();
    }
}

// ==========================================================================
// HslArea
// ==========================================================================

HslArea::HslArea(Type aType, int aCode) :
    iPrivate(new Private(aType, aCode))
{
}

HslArea::HslArea(const HslArea& aArea) :
    iPrivate(aArea.iPrivate)
{
    if (iPrivate) {
        iPrivate->iRef.ref();
    }
}

HslArea::HslArea() :
    iPrivate(NULL)
{
}

HslArea::~HslArea()
{
    if (iPrivate && !iPrivate->iRef.deref()) {
        delete iPrivate;
    }
}

HslArea& HslArea::operator = (const HslArea& aArea)
{
    if (iPrivate != aArea.iPrivate) {
        if (iPrivate && !iPrivate->iRef.deref()) {
            delete iPrivate;
        }
        iPrivate = aArea.iPrivate;
        if (iPrivate) {
            iPrivate->iRef.ref();
        }
    }
    return *this;
}

bool HslArea::equals(const HslArea& aArea) const
{
    if (iPrivate == aArea.iPrivate) {
        return true;
    } else if (iPrivate && aArea.iPrivate) {
        return iPrivate->iType == aArea.iPrivate->iType &&
               iPrivate->iCode == aArea.iPrivate->iCode;
    } else {
        return false;
    }
}

bool HslArea::valid() const
{
    return iPrivate != Q_NULLPTR;
}

HslArea::Type HslArea::type() const
{
    return iPrivate ? iPrivate->iType : UnknownArea;
}

int HslArea::code() const
{
    return iPrivate ? iPrivate->iCode : -1;
}

const QString HslArea::name() const
{
    return iPrivate ? iPrivate->iName : QString();
}

QDebug operator<<(QDebug aDebug, const HslArea& aArea)
{
    if (aArea.valid()) {
        QDebugStateSaver saver(aDebug);
        aDebug.nospace() << "HslArea(";
        const QString name(aArea.name());
        if (name.isEmpty()) {
            aDebug << aArea.type() << ", " << aArea.code();
        } else {
            aDebug << name;
        }
        aDebug << ")";
    } else {
        aDebug << "HslArea()";
    }
    return aDebug;
}

/*
 * Copyright (C) 2019-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2019-2020 Jolla Ltd.
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

#include "HslCardHistory.h"
#include "HslData.h"
#include "Util.h"

#include "HarbourDebug.h"

// Model roles
#define MODEL_ROLES_(first,role,last) \
    first(TransactionType,transactionType) \
    role(BoardingTime,boardingTime) \
    role(TicketPrice,ticketPrice) \
    role(GroupSize,groupSize) \
    last(RemainingValue,remainingValue)

#define MODEL_ROLES(role) \
    MODEL_ROLES_(role,role,role)

// ==========================================================================
// HslCardHistory::ModelData
// ==========================================================================

class HslCardHistory::ModelData
{
public:
    typedef QList<ModelData*> List;
    enum Role {
        #define FIRST(X,x) FirstRole = Qt::UserRole, X##Role = FirstRole,
        #define ROLE(X,x) X##Role,
        #define LAST(X,x) X##Role, LastRole = X##Role
        MODEL_ROLES_(FIRST,ROLE,LAST)
        #undef FIRST
        #undef ROLE
        #undef LAST
    };

    ModelData(TransactionType, QDateTime, int, int, int);

    QVariant get(Role) const;

public:
    TransactionType iTransactionType;
    QDateTime iBoardingTime;
    int iTicketPrice;
    int iGroupSize;
    int iRemainingValue;
};

HslCardHistory::ModelData::ModelData(
    TransactionType aType,
    QDateTime aBoardingTime,
    int aTicketPrice,
    int aGroupSize,
    int aRemainingValue) :
    iTransactionType(aType),
    iBoardingTime(aBoardingTime),
    iTicketPrice(aTicketPrice),
    iGroupSize(aGroupSize),
    iRemainingValue(aRemainingValue)
{
}

QVariant
HslCardHistory::ModelData::get(
    Role aRole) const
{
    switch (aRole) {
    case TransactionTypeRole: return iTransactionType;
    case BoardingTimeRole: return iBoardingTime;
    case TicketPriceRole: return iTicketPrice;
    case GroupSizeRole: return iGroupSize;
    case RemainingValueRole: return iRemainingValue;
    }
    return QVariant();
}

// ==========================================================================
// HslCardHistory::Private
// ==========================================================================

class HslCardHistory::Private
{
public:
    enum { ENTRY_SIZE = 12 };

    Private();
    ~Private();

    void setHexData(QString);
    ModelData* dataAt(int) const;

public:
    QString iHexData;
    ModelData::List iData;
};

HslCardHistory::Private::Private()
{}

HslCardHistory::Private::~Private()
{
    qDeleteAll(iData);
}

void
HslCardHistory::Private::setHexData(
    QString aHexData)
{
    QByteArray hexData(aHexData.toLatin1());
    const QByteArray bytes(QByteArray::fromHex(hexData));

    HDEBUG(hexData.constData());
    iHexData = aHexData;
    qDeleteAll(iData);
    iData.clear();
    if (!bytes.isEmpty()) {
        const GUtilData data = Util::toData(bytes);

        // History block layout (12 bytes each):
        //
        // +=========================================================+
        // | Byte/Bit | Bit   | Entry | Description                  |
        // | offset   | count | type  |                              |
        // +==========+=======+=======+==============================+
        // | 0/0      | 1     | uint  | Type (0 = check, 1 = charge) |
        // | 0/1      | 14    | date  | Boarding date                |
        // | 1/7      | 11    | time  | Boarding time                |
        // | 3/2      | 14    | date  | Transfer end date            |
        // | 5/0      | 11    | time  | Transfer end time            |
        // | 6/3      | 14    | uint  | Ticket fare (cents)          |
        // | 8/1      | 6     | uint  | Group size                   |
        // | 8/7      | 20    | uint  | Remaining value (cents)      |
        // | 11/3     | 5     | -     | Reserved                     |
        // +=========================================================+
        HASSERT(!(data.size % ENTRY_SIZE));
        const int n = data.size / ENTRY_SIZE;
        HDEBUG(n << "history entries:");
        for (int i = n - 1; i >= 0; i--) {
            const int off = i * ENTRY_SIZE;
            HDEBUG("Entry #" << (i + 1));
            const int type = HslData::getInt(&data, off, 0, 1);
            HDEBUG("  TransactionType =" << type);
            const QDate boardingDate(HslData::getDate(&data, off, 1));
            HDEBUG("  BoardingDate =" << HslData::getInt(&data, off, 1, HslData::DATE_BITS) << boardingDate);
            const QTime boardingTime(HslData::getTime(&data, off + 1, 7));
            HDEBUG("  BoardingTime =" << HslData::getInt(&data, off + 1, 7, HslData::TIME_BITS) << boardingTime);
            const int ticketPrice = HslData::getInt(&data, off + 6, 3, 14);
            HDEBUG("  TicketFare =" << ticketPrice);
            const int groupSize = HslData::getInt(&data, off + 8, 1, 6);
            HDEBUG("  GroupSize =" << groupSize);
            const int remainingValue = HslData::getInt(&data, off + 8, 7, 20);
            HDEBUG("  RemainingValue =" << remainingValue);
            // 0=Kauden leimaus, 1=Arvon veloitus
            iData.append(new ModelData((type == 0) ? TransactionBoarding :
                (type == 1) ? TransactionPurchase : TransactionUnknown,
                QDateTime(boardingDate, boardingTime, Util::FINLAND_TIMEZONE),
                ticketPrice, groupSize, remainingValue));
        }
    }
}

inline
HslCardHistory::ModelData*
HslCardHistory::Private::dataAt(
    int aIndex) const
{
    if (aIndex >= 0 && aIndex < iData.count()) {
        return iData.at(aIndex);
    } else {
        return NULL;
    }
}

// ==========================================================================
// HslCardHistory
// ==========================================================================

HslCardHistory::HslCardHistory(
    QObject* aParent) :
    QAbstractListModel(aParent),
    iPrivate(new Private)
{}

HslCardHistory::~HslCardHistory()
{
    delete iPrivate;
}

QString
HslCardHistory::data() const
{
    return iPrivate->iHexData;
}

void
HslCardHistory::setData(
    QString aData)
{
    const QString data(aData.toLower());

    if (iPrivate->iHexData != data) {
        const ModelData::List prevData(iPrivate->iData);
        const int prevCount = prevData.count();

        iPrivate->iData.clear();
        iPrivate->setHexData(data);

        // All this just to avoid resetting the entire model
        // which resets view position too.
        const ModelData::List newData(iPrivate->iData);
        const int count = iPrivate->iData.count();

        iPrivate->iData = prevData;
        if (count < prevCount) {
            beginRemoveRows(QModelIndex(), count, prevCount - 1);
            iPrivate->iData = newData;
            endRemoveRows();
            if (count > 0) {
                Q_EMIT dataChanged(index(0), index(count - 1));
            }
        } else if (count > prevCount) {
            beginInsertRows(QModelIndex(), prevCount, count - 1);
            iPrivate->iData = newData;
            endInsertRows();
            if (prevCount > 0) {
                Q_EMIT dataChanged(index(0), index(prevCount - 1));
            }
        } else if (count > 0) {
            iPrivate->iData = newData;
            Q_EMIT dataChanged(index(0), index(count - 1));
        }
        qDeleteAll(prevData);
        Q_EMIT historyChanged();
    }
}

QHash<int,QByteArray>
HslCardHistory::roleNames() const
{
    QHash<int,QByteArray> roles;
    #define ROLE(X,x) roles.insert(ModelData::X##Role, #x);
    MODEL_ROLES(ROLE)
    #undef ROLE
    return roles;
}

int
HslCardHistory::rowCount(
    const QModelIndex& aParent) const
{
    return iPrivate->iData.count();
}

QVariant
HslCardHistory::data(
    const QModelIndex& aIndex,
    int aRole) const
{
    ModelData* data = iPrivate->dataAt(aIndex.row());
    return data ? data->get((ModelData::Role)aRole) : QVariant();
}

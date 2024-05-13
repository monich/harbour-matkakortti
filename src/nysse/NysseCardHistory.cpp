/*
 * Copyright (C) 2020-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2020 Jolla Ltd.
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

#include "NysseCardHistory.h"
#include "NysseUtil.h"
#include "Util.h"

#include "HarbourDebug.h"

// Model roles
#define MODEL_ROLES_(first,role,last) \
    first(TransactionType,transactionType) \
    role(TransactionTime,transactionTime) \
    role(PassengerCount,passengerCount) \
    last(MoneyAmount,moneyAmount)

#define MODEL_ROLES(role) \
    MODEL_ROLES_(role,role,role)

// ==========================================================================
// NysseCardHistory::ModelData
// ==========================================================================

class NysseCardHistory::ModelData
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

    ModelData(TransactionType, QDateTime, uint, uint);

    QVariant get(Role) const;

public:
    const TransactionType iTransactionType;
    const QDateTime iTransactionTime;
    const uint iPassengerCount;
    const uint iMoneyAmount;
};

NysseCardHistory::ModelData::ModelData(
    TransactionType aType,
    QDateTime aTransactionTime,
    uint aPassengerCount,
    uint aMoneyAmount) :
    iTransactionType(aType),
    iTransactionTime(aTransactionTime),
    iPassengerCount(aPassengerCount),
    iMoneyAmount(aMoneyAmount)
{
}

QVariant
NysseCardHistory::ModelData::get(
    Role aRole) const
{
    switch (aRole) {
    case TransactionTypeRole: return iTransactionType;
    case TransactionTimeRole: return iTransactionTime;
    case PassengerCountRole: return iPassengerCount;
    case MoneyAmountRole: return iMoneyAmount;
    }
    return QVariant();
}

// ==========================================================================
// NysseCardHistory::Private
// ==========================================================================

class NysseCardHistory::Private
{
public:
    enum { ENTRY_SIZE = 16 };

    Private();
    ~Private();

    void setHexData(const QString);
    const ModelData* dataAt(int) const;

public:
    QString iHexData;
    ModelData::List iData;
};

NysseCardHistory::Private::Private()
{
}

NysseCardHistory::Private::~Private()
{
    qDeleteAll(iData);
}

void
NysseCardHistory::Private::setHexData(
    const QString aHexData)
{
    iHexData = aHexData;
    qDeleteAll(iData);
    iData.clear();

    HDEBUG(qPrintable(aHexData));
    const QByteArray bytes(QByteArray::fromHex(aHexData.toLatin1()));
    HASSERT(!(bytes.size() % ENTRY_SIZE));
    const int n = bytes.size() / ENTRY_SIZE;
    HDEBUG(n << "history entries:");
    for (int i = n - 1; i >= 0; i--) {
        const guint8* block = (guint8*) bytes.constData() + i * ENTRY_SIZE;

        // History block layout:
        //
        // +===========================================================+
        // | Offset | Size | Description                               |
        // +===========================================================+
        // | 0      | 2    | Date                                      |
        // | 2      | 1    | Minutes since last validation             |
        // | 3      | 3    | ???                                       |
        // | 6      | 2    | Time + Transaction type (low nibble)      |
        // | 8      | 2    | Transaction amount (in cents)             |
        // | 10     | 2    | Route + something (low 2 bits)            |
        // | 12     | 1    | Transaction type (part 2)                 |
        // | 13     | 1    | Passenger count (high nibble) + something |
        // | 14     | 2    | ???                                       |
        // +===========================================================+
        //
        HDEBUG("Entry #" << (i + 1) << ":" <<
            QByteArray((char*)block, ENTRY_SIZE).toHex().constData());
        const guint typeCode = (((guint)(block[6] & 0x0f)) << 8) + block[12];
        TransactionType type;
        // Not so sure about these...
        switch (typeCode) {
        case 0x100: type = TransactionIssue; break;
        case 0x300: type = TransactionCharge; break;
        case 0x301: type = TransactionDeposit; break;
        case 0x54a: type = TransactionPurchase; break;
        case 0x548:
        case 0xb48: type = TransactionValidation; break;
        default:    type = TransactionUnknown; break;
        }
        HDEBUG("  Type =" << hex << typeCode << "(" << dec  << type << ")");
        QDateTime time(NysseUtil::toDateTime(Util::uint16le(block + 0),
            Util::uint16le(block + 6) >> 4));
        HDEBUG("  Time =" << time);
        const uint count = block[13] >> 4;
        HDEBUG("  Count =" << count);
        const uint amount = Util::uint16le(block + 8);
        HDEBUG("  MoneyAmount =" << amount);
        iData.append(new ModelData(type, time, count, amount));
    }
}

inline
const NysseCardHistory::ModelData*
NysseCardHistory::Private::dataAt(
    int aIndex) const
{
    if (aIndex >= 0 && aIndex < iData.count()) {
        return iData.at(aIndex);
    } else {
        return NULL;
    }
}

// ==========================================================================
// NysseCardHistory
// ==========================================================================

NysseCardHistory::NysseCardHistory(
    QObject* aParent) :
    QAbstractListModel(aParent),
    iPrivate(new Private)
{
}

NysseCardHistory::~NysseCardHistory()
{
    delete iPrivate;
}

const QString
NysseCardHistory::data() const
{
    return iPrivate->iHexData;
}

void
NysseCardHistory::setData(
    const QString aData)
{
    QString data(aData.toLower());
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
                QAbstractListModel::dataChanged(index(0), index(count - 1));
            }
        } else if (count > prevCount) {
            beginInsertRows(QModelIndex(), prevCount, count - 1);
            iPrivate->iData = newData;
            endInsertRows();
            if (prevCount > 0) {
                QAbstractListModel::dataChanged(index(0), index(prevCount - 1));
            }
        } else if (count > 0) {
            iPrivate->iData = newData;
            QAbstractListModel::dataChanged(index(0), index(count - 1));
        }
        qDeleteAll(prevData);
        Q_EMIT dataChanged();
    }
}

QHash<int,QByteArray>
NysseCardHistory::roleNames() const
{
    QHash<int,QByteArray> roles;
#define ROLE(X,x) roles.insert(ModelData::X##Role, #x);
MODEL_ROLES(ROLE)
#undef ROLE
    return roles;
}

int
NysseCardHistory::rowCount(
    const QModelIndex&) const
{
    return iPrivate->iData.count();
}

QVariant
NysseCardHistory::data(
    const QModelIndex& aIndex,
    int aRole) const
{
    const ModelData* data = iPrivate->dataAt(aIndex.row());
    return data ? data->get((ModelData::Role)aRole) : QVariant();
}

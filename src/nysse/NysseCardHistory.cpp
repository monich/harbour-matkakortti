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

#include <gutil_misc.h>

#include "NysseCardHistory.h"
#include "NysseUtil.h"
#include "Util.h"

#include "HarbourDebug.h"

// Model roles
#define MODEL_ROLES_(first,role,last) \
    first(TransactionType,transactionType) \
    role(TransactionTime,transactionTime) \
    last(MoneyAmount,moneyAmount)

#define MODEL_ROLES(role) \
    MODEL_ROLES_(role,role,role)

// ==========================================================================
// NysseCardHistory::ModelData
// ==========================================================================

class NysseCardHistory::ModelData {
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

    ModelData(TransactionType aType, QDateTime aTransactionTime,
        int aMoneyAmount);

    QVariant get(Role aRole) const;

public:
    TransactionType iTransactionType;
    QDateTime iTransactionTime;
    int iMoneyAmount;
};

NysseCardHistory::ModelData::ModelData(TransactionType aType,
    QDateTime aTransactionTime, int aMoneyAmount) :
    iTransactionType(aType),
    iTransactionTime(aTransactionTime),
    iMoneyAmount(aMoneyAmount)
{
}

QVariant NysseCardHistory::ModelData::get(Role aRole) const
{
    switch (aRole) {
    case TransactionTypeRole: return iTransactionType;
    case TransactionTimeRole: return iTransactionTime;
    case MoneyAmountRole: return iMoneyAmount;
    }
    return QVariant();
}

// ==========================================================================
// NysseCardHistory::Private
// ==========================================================================

class NysseCardHistory::Private {
public:
    enum { ENTRY_SIZE = 16 };

    Private();
    ~Private();

    void setHexData(QString aHexData);
    ModelData* dataAt(int aIndex) const;

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

void NysseCardHistory::Private::setHexData(QString aHexData)
{
    iHexData = aHexData;
    QByteArray hexData(aHexData.toLatin1());
    HDEBUG(hexData.constData());
    GBytes* bytes = gutil_hex2bytes(hexData.constData(), hexData.size());
    qDeleteAll(iData);
    iData.clear();
    if (bytes) {
        GUtilData data;
        gutil_data_from_bytes(&data, bytes);
        HASSERT(!(data.size % ENTRY_SIZE));
        const int n = data.size / ENTRY_SIZE;
        HDEBUG(n << "history entries:");
        for (int i = n - 1; i >= 0; i--) {
            const guint8* entry = data.bytes + i * ENTRY_SIZE;
            HDEBUG("Entry #" << (i + 1));
            const guint32 typeCode = Util::uint32be(entry + 2);
            TransactionType type;
            // No so sure about these...
            switch (typeCode) {
            case 0x0000d417: type = TransactionIssue; break;
            case 0x00001018: type = TransactionCharge; break;
            case 0x00004c04: type = TransactionDeposit; break;
            case 0x000bde07: type = TransactionBoarding; break;
            default: type = TransactionUnknown; break;
            }
            HDEBUG("  Type =" << typeCode << "(" << type << ")");
            QDateTime time(NysseUtil::toDateTime(Util::uint16le(entry + 0),
                Util::uint16le(entry + 6)));
            HDEBUG("  Time =" << time);
            const uint amount = Util::uint16le(entry + 8);
            HDEBUG("  MoneyAmount =" << amount);
            iData.append(new ModelData(type, time, amount));
        }
        g_bytes_unref(bytes);
    }
}

inline NysseCardHistory::ModelData* NysseCardHistory::Private::dataAt(int aIndex) const
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

NysseCardHistory::NysseCardHistory(QObject* aParent) :
    QAbstractListModel(aParent),
    iPrivate(new Private)
{
}

NysseCardHistory::~NysseCardHistory()
{
    delete iPrivate;
}

QString NysseCardHistory::data() const
{
    return iPrivate->iHexData;
}

void NysseCardHistory::setData(QString aData)
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

QHash<int,QByteArray> NysseCardHistory::roleNames() const
{
    QHash<int,QByteArray> roles;
#define ROLE(X,x) roles.insert(ModelData::X##Role, #x);
MODEL_ROLES(ROLE)
#undef ROLE
    return roles;
}

int NysseCardHistory::rowCount(const QModelIndex& aParent) const
{
    return iPrivate->iData.count();
}

QVariant NysseCardHistory::data(const QModelIndex& aIndex, int aRole) const
{
    ModelData* data = iPrivate->dataAt(aIndex.row());
    return data ? data->get((ModelData::Role)aRole) : QVariant();
}

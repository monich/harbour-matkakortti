/*
 * Copyright (C) 2019-2024 Slava Monich <slava@monich.com>
 * Copyright (C) 2019-2022 Jolla Ltd.
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

#include "nfcdc_isodep.h"

#include "HslCard.h"
#include "HslCardAppInfo.h"
#include "HslCardEticket.h"
#include "HslCardHistory.h"
#include "HslCardPeriodPass.h"
#include "HslCardStoredValue.h"
#include "HslData.h"
#include "Util.h"

#include "HarbourDebug.h"
#include "HarbourUtil.h"

// ==========================================================================
// HslCard::Private
// ==========================================================================

#define SELECT_RESPONSE_SLOT selectResponse
#define READ_APPINFO_RESPONSE_SLOT readAppInfoResponse
#define READ_PERIODPASS_RESPONSE_SLOT readPeriodPassResponse
#define READ_STOREDVALUE_RESPONSE_SLOT readStoredValueResponse
#define READ_ETICKET_RESPONSE_SLOT readEticketResponse
#define READ_HISTORY_RESPONSE_SLOT readHistoryResp

class HslCard::Private :
    public QObject
{
    Q_OBJECT

public:
    Private(HslCard*);

    HslCard* parentObject() const;
    void transmit(const NfcIsoDepApdu*, const char*);
    void readFailed();
    void readSucceeded();

    static TravelCardImpl* newTravelCard(QString, QObject*);
    static void registerTypes(const char*, int, int);

    static const QString PAGE_URL;

    static const QString APP_INFO_KEY;
    static const QString PERIOD_PASS_KEY;
    static const QString STORED_VALUE_KEY;
    static const QString ETICKET_KEY;
    static const QString HISTORY_KEY;

    static const uchar SELECT_CMD_DATA[];
    static const uchar READ_APPINFO_CMD_DATA[];
    static const uchar READ_PERIODPASS_CMD_DATA[];
    static const uchar READ_STOREDVALUE_CMD_DATA[];
    static const uchar READ_ETICKET_CMD_DATA[];
    static const uchar READ_HISTORY_CMD_DATA[];

    static const NfcIsoDepApdu SELECT_CMD;
    static const NfcIsoDepApdu READ_APPINFO_CMD;
    static const NfcIsoDepApdu READ_PERIODPASS_CMD;
    static const NfcIsoDepApdu READ_STOREDVALUE_CMD;
    static const NfcIsoDepApdu READ_ETICKET_CMD;
    static const NfcIsoDepApdu READ_HISTORY_CMD;
    static const NfcIsoDepApdu READ_MORE_CMD;

    static const uint APPINFO_SIZE = 11;
    static const uint PERIODPASS_SIZE = 35;
    static const uint STOREDVALUE_SIZE = 13;
    static const uint ETICKET_SIZE = 45;

    static const uint SW_OK = NFC_ISODEP_SW(0x91, 0x00);
    static const uint SW_MORE = NFC_ISODEP_SW(0x91, 0xaf);

private Q_SLOTS:
    void SELECT_RESPONSE_SLOT(const GUtilData*, uint, const GError*);
    void READ_APPINFO_RESPONSE_SLOT(const GUtilData*, uint, const GError*);
    void READ_PERIODPASS_RESPONSE_SLOT(const GUtilData*, uint, const GError*);
    void READ_STOREDVALUE_RESPONSE_SLOT(const GUtilData*, uint, const GError*);
    void READ_ETICKET_RESPONSE_SLOT(const GUtilData*, uint, const GError*);
    void READ_HISTORY_RESPONSE_SLOT(const GUtilData*, uint, const GError*);

public:
    QByteArray iAppInfoData;
    QByteArray iPeriodPassData;
    QByteArray iStoredValueData;
    QByteArray iEticketData;
    QByteArray iHistoryData;
};

const QString HslCard::Private::PAGE_URL("hsl/HslPage.qml");

const QString HslCard::Private::APP_INFO_KEY("appInfo");
const QString HslCard::Private::PERIOD_PASS_KEY("periodPass");
const QString HslCard::Private::STORED_VALUE_KEY("storedValue");
const QString HslCard::Private::ETICKET_KEY("eTicket");
const QString HslCard::Private::HISTORY_KEY("history");

const uchar HslCard::Private::SELECT_CMD_DATA[] = {
    0x14, 0x20, 0xef
};
const NfcIsoDepApdu HslCard::Private::SELECT_CMD = {
    0x90, 0x5a, 0x00, 0x00,
    { SELECT_CMD_DATA, sizeof(SELECT_CMD_DATA) },
    0x100
};

const uchar HslCard::Private::READ_APPINFO_CMD_DATA[] = {
    0x08, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00
};
const NfcIsoDepApdu HslCard::Private::READ_APPINFO_CMD = {
    0x90, 0xbd, 0x00, 0x00,
    { READ_APPINFO_CMD_DATA, sizeof(READ_APPINFO_CMD_DATA) },
    0x100
};

const uchar HslCard::Private::READ_PERIODPASS_CMD_DATA[] = {
    0x01, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00
};
const NfcIsoDepApdu HslCard::Private::READ_PERIODPASS_CMD = {
    0x90, 0xbd, 0x00, 0x00,
    { READ_PERIODPASS_CMD_DATA, sizeof(READ_PERIODPASS_CMD_DATA) },
    0x100
};

const uchar HslCard::Private::READ_STOREDVALUE_CMD_DATA[] = {
    0x02, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00
};
const NfcIsoDepApdu HslCard::Private::READ_STOREDVALUE_CMD = {
    0x90, 0xbd, 0x00, 0x00,
    { READ_STOREDVALUE_CMD_DATA, sizeof(READ_STOREDVALUE_CMD_DATA) },
    0x100
};

const uchar HslCard::Private::READ_ETICKET_CMD_DATA[] = {
    0x03, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00
};
const NfcIsoDepApdu HslCard::Private::READ_ETICKET_CMD = {
    0x90, 0xbd, 0x00, 0x00,
    { READ_ETICKET_CMD_DATA, sizeof(READ_ETICKET_CMD_DATA) },
    0x100
};

const uchar HslCard::Private::READ_HISTORY_CMD_DATA[] = {
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const NfcIsoDepApdu HslCard::Private::READ_HISTORY_CMD = {
    0x90, 0xbb, 0x00, 0x00,
    { READ_HISTORY_CMD_DATA, sizeof(READ_HISTORY_CMD_DATA) },
    0x100
};

const NfcIsoDepApdu HslCard::Private::READ_MORE_CMD = {
    0x90, 0xaf, 0x00, 0x00,
    { NULL, 0 },
    0x100
};

HslCard::Private::Private(
    HslCard* aParent) :
    QObject(aParent)
{
}

inline
HslCard*
HslCard::Private::parentObject() const
{
    return qobject_cast<HslCard*>(parent());
}

inline
void
HslCard::Private::transmit(
    const NfcIsoDepApdu* aApdu,
    const char* aMethod)
{
    parentObject()->transmit(aApdu, this, aMethod);
}

void
HslCard::Private::readSucceeded()
{
    QVariantMap cardInfo;
    cardInfo.insert(Util::CARD_TYPE_KEY, Desc.iName);
    cardInfo.insert(APP_INFO_KEY, HarbourUtil::toHex(iAppInfoData));
    cardInfo.insert(PERIOD_PASS_KEY, HarbourUtil::toHex(iPeriodPassData));
    cardInfo.insert(STORED_VALUE_KEY, HarbourUtil::toHex(iStoredValueData));
    cardInfo.insert(ETICKET_KEY, HarbourUtil::toHex(iEticketData));
    cardInfo.insert(HISTORY_KEY, HarbourUtil::toHex(iHistoryData));
    parentObject()->success(PAGE_URL, cardInfo);
}

void
HslCard::Private::readFailed()
{
    parentObject()->failure(IoError);
}

void
HslCard::Private::READ_HISTORY_RESPONSE_SLOT(
    const GUtilData* aResponse,
    uint aSw,
    const GError* aError)
{
    if (!aError) {
        iHistoryData += Util::toByteArray(aResponse);
        if (aSw == SW_OK) {
            HDEBUG("READ_HISTORY ok" << iHistoryData.size() << "bytes");
            readSucceeded();
        } else if (aSw == SW_MORE) {
            HDEBUG("READ_HISTORY ok" << iHistoryData.size() << "bytes");
            HDEBUG("READ_MORE");
            transmit(&READ_MORE_CMD, QT_STRINGIFY(READ_HISTORY_RESPONSE_SLOT));
        } else {
            HDEBUG("READ_HISTORY unexpected status" << hex << aSw);
            readFailed();
        }
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("READ_HISTORY error" << aError->message);
        } else {
            HDEBUG("READ_HISTORY error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        readFailed();
    }
}

void
HslCard::Private::READ_ETICKET_RESPONSE_SLOT(
    const GUtilData* aResponse,
    uint aSw,
    const GError* aError)
{
    if (!aError && aSw == SW_OK && aResponse->size == ETICKET_SIZE) {
        HDEBUG("READ_ETICKET" << aResponse->size << "bytes");
        iEticketData = Util::toByteArray(aResponse);
        HDEBUG("READ_HISTORY");
        iHistoryData.resize(0);
        transmit(&READ_HISTORY_CMD, QT_STRINGIFY(READ_HISTORY_RESPONSE_SLOT));
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("READ_ETICKET error" << aError->message);
        } else if (aSw == SW_OK) {
            HDEBUG("READ_ETICKET unexpected size" << aResponse->size);
        } else {
            HDEBUG("READ_ETICKET error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        readFailed();
    }
}

void
HslCard::Private::READ_STOREDVALUE_RESPONSE_SLOT(
    const GUtilData* aResponse,
    uint aSw,
    const GError* aError)
{
    if (!aError && aSw == SW_OK && aResponse->size == STOREDVALUE_SIZE) {
        HDEBUG("READ_STOREDVALUE" << aResponse->size << "bytes");
        iStoredValueData = Util::toByteArray(aResponse);
        HDEBUG("READ_ETICKET");
        transmit(&READ_ETICKET_CMD, QT_STRINGIFY(READ_ETICKET_RESPONSE_SLOT));
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("READ_STOREDVALUE error" << aError->message);
        } else if (aSw == SW_OK) {
            HDEBUG("READ_STOREDVALUE unexpected size" << aResponse->size);
        } else {
            HDEBUG("READ_STOREDVALUE error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        readFailed();
    }
}

void
HslCard::Private::READ_PERIODPASS_RESPONSE_SLOT(
    const GUtilData* aResponse,
    uint aSw,
    const GError* aError)
{
    if (!aError && aSw == SW_OK && aResponse->size == PERIODPASS_SIZE) {
        HDEBUG("READ_PERIODPASS" << aResponse->size << "bytes");
        iPeriodPassData = Util::toByteArray(aResponse);
        HDEBUG("READ_STOREDVALUE");
        transmit(&READ_STOREDVALUE_CMD, QT_STRINGIFY(READ_STOREDVALUE_RESPONSE_SLOT));
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("READ_PERIODPASS error" << aError->message);
        } else if (aSw == SW_OK) {
            HDEBUG("READ_PERIODPASS unexpected size" << aResponse->size);
        } else {
            HDEBUG("READ_PERIODPASS error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        readFailed();
    }
}

void
HslCard::Private::READ_APPINFO_RESPONSE_SLOT(
    const GUtilData* aResponse,
    uint aSw,
    const GError* aError)
{
    if (!aError && aSw == SW_OK && aResponse->size == APPINFO_SIZE) {
        HDEBUG("READ_APPINFO" << aResponse->size << "bytes");
        iAppInfoData = Util::toByteArray(aResponse);
        HDEBUG("READ_PERIODPASS");
        transmit(&READ_PERIODPASS_CMD, QT_STRINGIFY(READ_PERIODPASS_RESPONSE_SLOT));
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("READ_APPINFO error" << aError->message);
        } else if (aSw == SW_OK) {
            HDEBUG("READ_APPINFO unexpected size" << aResponse->size);
        } else {
            HDEBUG("READ_APPINFO error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        readFailed();
    }
}

void
HslCard::Private::SELECT_RESPONSE_SLOT(
    const GUtilData*,
    uint aSw,
    const GError* aError)
{
    if (!aError && aSw == SW_OK) {
        HDEBUG("SELECT ok");
        HDEBUG("READ_APPINFO");
        transmit(&READ_APPINFO_CMD, QT_STRINGIFY(READ_APPINFO_RESPONSE_SLOT));
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("SELECT error" << aError->message);
        } else {
            HDEBUG("SELECT error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        readFailed();
    }
}

// ==========================================================================
// HslCard
// ==========================================================================

HslCard::HslCard(
    QString aPath,
    QObject* aParent) :
    TravelCardIsoDep(aPath, aParent),
    iPrivate(new Private(this))
{}

void
HslCard::startIo()
{
    HDEBUG("SELECT");
    transmit(&Private::SELECT_CMD, iPrivate, QT_STRINGIFY(SELECT_RESPONSE_SLOT));
}

// ==========================================================================
// HslCard::Desc
// ==========================================================================

#define REGISTER_META_TYPE(t) qRegisterMetaType<t>(#t)
#define REGISTER_TYPE(t,uri,v1,v2) qmlRegisterType<t>(uri,v1,v2,#t)
#define REGISTER_SINGLETON_TYPE(t,uri,v1,v2) \
    qmlRegisterSingletonType<t>(uri,v1,v2,#t,t::createSingleton)

TravelCardImpl*
HslCard::Private::newTravelCard(
    QString aPath,
    QObject* aParent)
{
    return new HslCard(aPath, aParent);
}

void
HslCard::Private::registerTypes(
    const char* aUri,
    int v1,
    int v2)
{
    REGISTER_META_TYPE(HslArea);
    REGISTER_TYPE(HslCardAppInfo, aUri, v1, v2);
    REGISTER_TYPE(HslCardEticket, aUri, v1, v2);
    REGISTER_TYPE(HslCardHistory, aUri, v1, v2);
    REGISTER_TYPE(HslCardPeriodPass, aUri, v1, v2);
    REGISTER_TYPE(HslCardStoredValue, aUri, v1, v2);
    REGISTER_SINGLETON_TYPE(HslData, aUri, v1, v2);
}

const TravelCardImpl::CardDesc HslCard::Desc = {
    QStringLiteral("HSL"),
    HslCard::Private::newTravelCard,
    HslCard::Private::registerTypes
};

#include "HslCard.moc"

/*
 * Copyright (C) 2019-2020 Jolla Ltd.
 * Copyright (C) 2019-2020 Slava Monich <slava.monich@jolla.com>
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

#include "nfcdc_isodep.h"
#include "nfcdc_tag.h"

#include "gutil_log.h"

#include "HslCard.h"
#include "HslCardAppInfo.h"
#include "HslCardEticket.h"
#include "HslCardHistory.h"
#include "HslCardPeriodPass.h"
#include "HslCardStoredValue.h"
#include "Util.h"

#include "HarbourDebug.h"

enum tag_events {
    TAG_EVENT_VALID,
    TAG_EVENT_PRESENT,
    TAG_EVENT_COUNT
};

// ==========================================================================
// HslCard::Private
// ==========================================================================

class HslCard::Private {
public:
    Private(QString aPath, HslCard* aParent);
    ~Private();

    void readDone();
    void readFailed();
    void readSucceeded();
    void startReadingIfReady();

    static TravelCardImpl* newTravelCard(QString aPath, QObject* aParent);
    static void registerTypes(const char* aUri, int v1, int v2);

    static void tagEventHandler(NfcTagClient*, NFC_TAG_PROPERTY, void*);
    static void isoDepEventHandler(NfcIsoDepClient*, NFC_ISODEP_PROPERTY, void*);
    static void tagLockResp(NfcTagClient*, NfcTagClientLock*, const GError*, void*);
    static void selectResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readAppInfoResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readPeriodPassResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readStoredValueResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readEticketResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readHistoryResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);

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

public:
    HslCard* iParent;
    NfcTagClient* iTag;
    NfcTagClientLock* iLock;
    NfcIsoDepClient* iIsoDep;
    gulong iTagEventId[TAG_EVENT_COUNT];
    gulong iIsoDepEventId[TAG_EVENT_COUNT];
    GCancellable* iCancel;
    QByteArray iAppInfoData;
    QByteArray iPeriodPassData;
    QByteArray iStoredValueData;
    QByteArray iEticketData;
    QByteArray iHistoryData;
};

const QString HslCard::Private::PAGE_URL("HslCardPage.qml");

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

HslCard::Private::Private(QString aPath, HslCard* aParent) :
    iParent(aParent),
    iTag(Q_NULLPTR),
    iLock(Q_NULLPTR),
    iIsoDep(Q_NULLPTR),
    iCancel(Q_NULLPTR)
{
    memset(iTagEventId, 0, sizeof(iTagEventId));
    memset(iIsoDepEventId, 0, sizeof(iIsoDepEventId));

    QByteArray bytes(aPath.toLatin1());
    const char* path = bytes.constData();

    iTag = nfc_tag_client_new(path);
    iTagEventId[TAG_EVENT_VALID] =
        nfc_tag_client_add_property_handler(iTag,
            NFC_TAG_PROPERTY_VALID, tagEventHandler, this);
    iTagEventId[TAG_EVENT_PRESENT] =
        nfc_tag_client_add_property_handler(iTag,
            NFC_TAG_PROPERTY_PRESENT, tagEventHandler, this);

    iIsoDep = nfc_isodep_client_new(path);
    startReadingIfReady();
    if (!iCancel) {
        iIsoDepEventId[TAG_EVENT_VALID] =
            nfc_isodep_client_add_property_handler(iIsoDep,
                NFC_ISODEP_PROPERTY_VALID, isoDepEventHandler, this);
        iIsoDepEventId[TAG_EVENT_PRESENT] =
            nfc_isodep_client_add_property_handler(iIsoDep,
                NFC_ISODEP_PROPERTY_PRESENT, isoDepEventHandler, this);
    }
}

HslCard::Private::~Private()
{
    if (iCancel) {
        g_cancellable_cancel(iCancel);
        g_object_unref(iCancel);
    }
    nfc_isodep_client_remove_all_handlers(iIsoDep, iIsoDepEventId);
    nfc_tag_client_remove_all_handlers(iTag, iTagEventId);
    nfc_isodep_client_unref(iIsoDep);
    nfc_tag_client_unref(iTag);
    nfc_tag_client_lock_unref(iLock);
}

void HslCard::Private::readDone()
{
    if (iCancel) {
        g_cancellable_cancel(iCancel);
        g_object_unref(iCancel);
        iCancel = Q_NULLPTR;
    }
    if (iLock) {
        nfc_tag_client_lock_unref(iLock);
        iLock = Q_NULLPTR;
    }
}

void HslCard::Private::readSucceeded()
{
    HDEBUG("Read done");
    readDone();
    QVariantMap cardInfo;
    cardInfo.insert(Util::CARD_TYPE_KEY, Desc.iName);
    cardInfo.insert(APP_INFO_KEY, Util::toHex(iAppInfoData));
    cardInfo.insert(PERIOD_PASS_KEY, Util::toHex(iPeriodPassData));
    cardInfo.insert(STORED_VALUE_KEY, Util::toHex(iStoredValueData));
    cardInfo.insert(ETICKET_KEY, Util::toHex(iEticketData));
    cardInfo.insert(HISTORY_KEY, Util::toHex(iHistoryData));
    QMetaObject::invokeMethod(iParent, "readDone",
        Q_ARG(QString, PAGE_URL),
        Q_ARG(QVariantMap, cardInfo));
}

void HslCard::Private::readFailed()
{
    HDEBUG("Read failed");
    QMetaObject::invokeMethod(iParent, "readFailed");
    readDone();
}

void HslCard::Private::readHistoryResp(NfcIsoDepClient*,
    const GUtilData* aResponse, guint aSw, const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError) {
        self->iHistoryData += Util::toByteArray(aResponse);
        if (aSw == SW_OK) {
            HDEBUG("READ_HISTORY ok" << self->iHistoryData.size() << "bytes");
            self->readSucceeded();
        } else if (aSw == SW_MORE) {
            HDEBUG("READ_HISTORY ok" << self->iHistoryData.size() << "bytes");
            HDEBUG("READ_MORE");
            nfc_isodep_client_transmit(self->iIsoDep, &READ_MORE_CMD,
                self->iCancel, readHistoryResp, self, Q_NULLPTR);
        } else {
            HDEBUG("READ_HISTORY unexpected status" << hex << aSw);
            self->readFailed();
        }
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("READ_HISTORY error" << aError->message);
        } else {
            HDEBUG("READ_HISTORY error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        self->readFailed();
    }
}

void HslCard::Private::readEticketResp(NfcIsoDepClient*,
    const GUtilData* aResponse, guint aSw, const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK && aResponse->size == ETICKET_SIZE) {
        HDEBUG("READ_ETICKET" << aResponse->size << "bytes");
        self->iEticketData = Util::toByteArray(aResponse);
        HDEBUG("READ_HISTORY");
        nfc_isodep_client_transmit(self->iIsoDep, &READ_HISTORY_CMD,
            self->iCancel, readHistoryResp, self, Q_NULLPTR);
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
        self->readFailed();
    }
}

void HslCard::Private::readStoredValueResp(NfcIsoDepClient*,
    const GUtilData* aResponse, guint aSw, const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK && aResponse->size == STOREDVALUE_SIZE) {
        HDEBUG("READ_STOREDVALUE" << aResponse->size << "bytes");
        self->iStoredValueData = Util::toByteArray(aResponse);
        HDEBUG("READ_ETICKET");
        nfc_isodep_client_transmit(self->iIsoDep, &READ_ETICKET_CMD,
            self->iCancel, readEticketResp, self, Q_NULLPTR);
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
        self->readFailed();
    }
}

void HslCard::Private::readPeriodPassResp(NfcIsoDepClient*,
    const GUtilData* aResponse, guint aSw, const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK && aResponse->size == PERIODPASS_SIZE) {
        HDEBUG("READ_PERIODPASS" << aResponse->size << "bytes");
        self->iPeriodPassData = Util::toByteArray(aResponse);
        HDEBUG("READ_STOREDVALUE");
        nfc_isodep_client_transmit(self->iIsoDep, &READ_STOREDVALUE_CMD,
            self->iCancel, readStoredValueResp, self, Q_NULLPTR);
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
        self->readFailed();
    }
}

void HslCard::Private::readAppInfoResp(NfcIsoDepClient*,
    const GUtilData* aResponse, guint aSw, const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK && aResponse->size == APPINFO_SIZE) {
        HDEBUG("READ_APPINFO" << aResponse->size << "bytes");
        self->iAppInfoData = Util::toByteArray(aResponse);
        HDEBUG("READ_PERIODPASS");
        nfc_isodep_client_transmit(self->iIsoDep, &READ_PERIODPASS_CMD,
            self->iCancel, readPeriodPassResp, self, Q_NULLPTR);
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
        self->readFailed();
    }
}

void HslCard::Private::selectResp(NfcIsoDepClient*, const GUtilData*,
    guint aSw, const GError* aError, void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK) {
        HDEBUG("SELECT ok");
        HDEBUG("READ_APPINFO");
        nfc_isodep_client_transmit(self->iIsoDep, &READ_APPINFO_CMD,
            self->iCancel, readAppInfoResp, self, Q_NULLPTR);
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("SELECT error" << aError->message);
        } else {
            HDEBUG("SELECT error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        self->readFailed();
    }
}

void HslCard::Private::tagLockResp(NfcTagClient*, NfcTagClientLock* aLock,
    const GError* aError, void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    HASSERT(!self->iLock);
    if (aLock) {
        self->iLock = nfc_tag_client_lock_ref(aLock);
    } else {
        // This is not fatal, try to continue
        HWARN("Failed to lock the tag:" << GERRMSG(aError));
    }
    HDEBUG("SELECT");
    nfc_isodep_client_transmit(self->iIsoDep, &SELECT_CMD, self->iCancel,
        selectResp, self, Q_NULLPTR);
}

void HslCard::Private::startReadingIfReady()
{
    if (iIsoDep->valid && iTag->valid) {
        if (iTag->present && iIsoDep->present && !iCancel) {
            iAppInfoData.clear();
            iPeriodPassData.clear();
            iStoredValueData.clear();
            iEticketData.clear();
            iHistoryData.clear();
            iCancel = g_cancellable_new();
            nfc_tag_client_acquire_lock(iTag, TRUE, iCancel, tagLockResp,
                this, Q_NULLPTR);
        } else if (!iIsoDep->present) {
            // Not an ISO-DEP card
            readFailed();
        }
    }
}

void HslCard::Private::isoDepEventHandler(NfcIsoDepClient*,
    NFC_ISODEP_PROPERTY, void* aPrivate)
{
    ((Private*)aPrivate)->startReadingIfReady();
}

void HslCard::Private::tagEventHandler(NfcTagClient*,
    NFC_TAG_PROPERTY, void* aPrivate)
{
    ((Private*)aPrivate)->startReadingIfReady();
}

// ==========================================================================
// HslCard::Desc
// ==========================================================================

TravelCardImpl* HslCard::Private::newTravelCard(QString aPath, QObject* aParent)
{
    return new HslCard(aPath, aParent);
}

void HslCard::Private::registerTypes(const char* aUri, int v1, int v2)
{
    qRegisterMetaType<HslArea>("HslArea");
    qmlRegisterType<HslCardAppInfo>(aUri, v1, v2, "HslCardAppInfo");
    qmlRegisterType<HslCardEticket>(aUri, v1, v2, "HslCardEticket");
    qmlRegisterType<HslCardHistory>(aUri, v1, v2, "HslCardHistory");
    qmlRegisterType<HslCardPeriodPass>(aUri, v1, v2, "HslCardPeriodPass");
    qmlRegisterType<HslCardStoredValue>(aUri, v1, v2, "HslCardStoredValue");
}

const TravelCardImpl::CardDesc HslCard::Desc = {
    "HSL",
    HslCard::Private::newTravelCard,
    HslCard::Private::registerTypes
};

// ==========================================================================
// HslCard
// ==========================================================================

HslCard::HslCard(QString aPath, QObject* aParent) :
    TravelCardImpl(aParent),
    iPrivate(new Private(aPath, this))
{
}

HslCard::~HslCard()
{
    delete iPrivate;
}

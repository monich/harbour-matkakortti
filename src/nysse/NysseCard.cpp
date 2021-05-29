/*
 * Copyright (C) 2020-2021 Jolla Ltd.
 * Copyright (C) 2019-2021 Slava Monich <slava@monich.com>
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

#include "NysseCardAppInfo.h"
#include "NysseCardBalance.h"
#include "NysseCardHistory.h"
#include "NysseCardOwnerInfo.h"
#include "NysseCardSeasonPass.h"
#include "NysseCard.h"
#include "Util.h"

#include "HarbourDebug.h"

#if HARBOUR_DEBUG
#  define REPORT_ERROR(name,sw,err) \
    ((void)((err) ? HDEBUG(name " error" << (err)->message) : \
    HDEBUG(name " error" << hex << sw)))
#else
#  define REPORT_ERROR(name,sw,err) ((void)0)
#endif // HARBOUR_DEBUG

enum tag_events {
    TAG_EVENT_VALID,
    TAG_EVENT_PRESENT,
    TAG_EVENT_COUNT
};

enum data_blocks {
    APP_INFO_BLOCK,
    OWNER_INFO_BLOCK,
    SEASON_PASS_BLOCK,
    HISTORY_BLOCK,
    BALANCE_BLOCK,
    BLOCK_COUNT
};

// ==========================================================================
// NysseCard::Private
// ==========================================================================

class NysseCard::Private {
public:
    struct DataBlock {
        const char* iName;
        const char* iKey;
        uint iExpectedSize;
        uint iRecordSize;
        const NfcIsoDepApdu* iPrepareApdu;
        const NfcIsoDepApdu* iReadApdu;
    };

    Private(QString aPath, NysseCard* aParent);
    ~Private();

    void readDone();
    void readFailed();
    void readSucceeded();
    void startReadingIfReady();
    void readNextBlock();
    const DataBlock* currentBlock();

    static TravelCardImpl* newTravelCard(QString aPath, QObject* aParent);
    static void registerTypes(const char* aUri, int v1, int v2);

    static void tagEventHandler(NfcTagClient*, NFC_TAG_PROPERTY, void*);
    static void isoDepEventHandler(NfcIsoDepClient*, NFC_ISODEP_PROPERTY, void*);
    static void tagLockResp(NfcTagClient*, NfcTagClientLock*, const GError*, void*);
    static void selectResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void prepareResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);

    static const QString PAGE_URL;

    static const DataBlock DATA_BLOCKS[];

    static const uchar SELECT_CMD_DATA[];
    static const uchar PREPARE_APP_INFO_CMD_DATA[];
    static const uchar READ_APP_INFO_CMD_DATA[];
    static const uchar PREPARE_OWNER_INFO_CMD_DATA[];
    static const uchar READ_OWNER_INFO_CMD_DATA[];
    static const uchar PREPARE_SEASON_PASS_CMD_DATA[];
    static const uchar READ_SEASON_PASS_CMD_DATA[];
    static const uchar PREPARE_HISTORY_CMD_DATA[];
    static const uchar READ_HISTORY_CMD_DATA[];
    static const uchar PREPARE_BALANCE_CMD_DATA[];

    static const NfcIsoDepApdu SELECT_CMD;
    static const NfcIsoDepApdu PREPARE_APP_INFO_CMD;
    static const NfcIsoDepApdu READ_APP_INFO_CMD;
    static const NfcIsoDepApdu PREPARE_OWNER_INFO_CMD;
    static const NfcIsoDepApdu READ_OWNER_INFO_CMD;
    static const NfcIsoDepApdu PREPARE_SEASON_PASS_CMD;
    static const NfcIsoDepApdu READ_SEASON_PASS_CMD;
    static const NfcIsoDepApdu PREPARE_BALANCE_CMD;
    static const NfcIsoDepApdu PREPARE_HISTORY_CMD;
    static const NfcIsoDepApdu READ_HISTORY_CMD;
    static const NfcIsoDepApdu READ_BALANCE_CMD;
    static const NfcIsoDepApdu READ_MORE_CMD;

    static const uint SW_OK = NFC_ISODEP_SW(0x91, 0x00);
    static const uint SW_MORE = NFC_ISODEP_SW(0x91, 0xaf);

public:
    NysseCard* iParent;
    NfcTagClient* iTag;
    NfcTagClientLock* iLock;
    NfcIsoDepClient* iIsoDep;
    GCancellable* iCancel;
    gulong iTagEventId[TAG_EVENT_COUNT];
    gulong iIsoDepEventId[TAG_EVENT_COUNT];
    QByteArray iData[BLOCK_COUNT];
    int iCurrentBlock;
};

const QString NysseCard::Private::PAGE_URL("nysse/NyssePage.qml");

//
// This is basically what Android app does:
//
// iso-dep -v 90 5a 00 00 0121ef 0100
// iso-dep -v 90 6f 00 00 '' 0100
// iso-dep -v 90 f5 00 00 04 0100
// iso-dep -v 90 bd 00 00 04000000000000 0100
// iso-dep -v 90 f5 00 00 05 0100
// iso-dep -v 90 bd 00 00 05000000000000 0100
// iso-dep -v 90 f5 00 00 02 0100
// iso-dep -v 90 bd 00 00 02000000000000 0100
// iso-dep -v 90 f5 00 00 06 0100
// iso-dep -v 90 bd 00 00 06000000000000 0100
// iso-dep -v 90 f5 00 00 03 0100
// iso-dep -v 90 bb 00 00 03000000000000 0100
// iso-dep -v 90 f5 00 00 01 0100
// iso-dep -v 90 6c 00 00 01 0100
// iso-dep -v 90 f5 00 00 07 0100
// iso-dep -v 90 bd 00 00 07000000000000 0100
//
// And "read more" command needs to be issued if we get 91AF:
//
// iso-dep -v 90 af 00 00 '' 0100
//

const NysseCard::Private::DataBlock NysseCard::Private::DATA_BLOCKS[] = {
    {
        "APP_INFO", "appInfo", 32, 0,
        &PREPARE_APP_INFO_CMD, &READ_APP_INFO_CMD
    },{
        "OWNER_INFO", "ownerInfo", 96, 0,
        &PREPARE_OWNER_INFO_CMD, &READ_OWNER_INFO_CMD
    },{
        "SEASON_PASS", "seasonPass", 96, 0,
        &PREPARE_SEASON_PASS_CMD, &READ_SEASON_PASS_CMD
    },{
        "HISTORY", "history", 0, 16,
        &PREPARE_HISTORY_CMD, &READ_HISTORY_CMD
    },{
        "BALANCE", "balance", 4, 0,
        &PREPARE_BALANCE_CMD, &READ_BALANCE_CMD
    }
};

const uchar NysseCard::Private::SELECT_CMD_DATA[] = {
    0x01, 0x21, 0xef
};
const NfcIsoDepApdu NysseCard::Private::SELECT_CMD = {
    0x90, 0x5a, 0x00, 0x00,
    { SELECT_CMD_DATA, sizeof(SELECT_CMD_DATA) },
    0x100
};

#define PREPARE_CMD(NAME,x) \
const uchar NysseCard::Private::PREPARE_##NAME##_CMD_DATA[] = { x }; \
const NfcIsoDepApdu NysseCard::Private::PREPARE_##NAME##_CMD = { \
    0x90, 0xf5, 0x00, 0x00, \
    { PREPARE_##NAME##_CMD_DATA, sizeof(PREPARE_##NAME##_CMD_DATA) }, \
    0x100 \
}
#define CARD_ITEM(NAME,x,cmd) PREPARE_CMD(NAME,x); \
const uchar NysseCard::Private::READ_##NAME##_CMD_DATA[] = { \
    x, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
}; \
const NfcIsoDepApdu NysseCard::Private::READ_##NAME##_CMD = { \
    0x90, cmd, 0x00, 0x00, \
   { READ_##NAME##_CMD_DATA, sizeof(READ_##NAME##_CMD_DATA) }, \
    0x100 \
}

CARD_ITEM(APP_INFO, 0x07, 0xbd);
CARD_ITEM(OWNER_INFO, 0x04, 0xbd);
CARD_ITEM(SEASON_PASS, 0x02, 0xbd);
CARD_ITEM(HISTORY, 0x03, 0xbb);

PREPARE_CMD(BALANCE, 0x01);
const NfcIsoDepApdu NysseCard::Private::READ_BALANCE_CMD = {
    0x90, 0x6c, 0x00, 0x00, // Can reuse PREPARE_BALANCE_CMD_DATA
   { PREPARE_BALANCE_CMD_DATA, sizeof(PREPARE_BALANCE_CMD_DATA) },
    0x100
};

const NfcIsoDepApdu NysseCard::Private::READ_MORE_CMD = {
    0x90, 0xaf, 0x00, 0x00,
    { NULL, 0 },
    0x100
};

NysseCard::Private::Private(QString aPath, NysseCard* aParent) :
    iParent(aParent),
    iTag(Q_NULLPTR),
    iLock(Q_NULLPTR),
    iIsoDep(Q_NULLPTR),
    iCancel(Q_NULLPTR),
    iCurrentBlock(-1)
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

NysseCard::Private::~Private()
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

void NysseCard::Private::readDone()
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

void NysseCard::Private::readSucceeded()
{
    HDEBUG("Read done");
    readDone();
    QVariantMap cardInfo;
    cardInfo.insert(Util::CARD_TYPE_KEY, Desc.iName);
    for (int i = 0; i < BLOCK_COUNT; i++) {
        cardInfo.insert(DATA_BLOCKS[i].iKey, Util::toHex(iData[i]));
    }
    QMetaObject::invokeMethod(iParent, "readDone",
        Q_ARG(QString, PAGE_URL),
        Q_ARG(QVariantMap, cardInfo));
}

void NysseCard::Private::readFailed()
{
    HDEBUG("Read failed");
    QMetaObject::invokeMethod(iParent, "readFailed");
    readDone();
}

const NysseCard::Private::DataBlock* NysseCard::Private::currentBlock()
{
    return (iCurrentBlock >= 0 && iCurrentBlock < BLOCK_COUNT) ?
        (DATA_BLOCKS + iCurrentBlock) : Q_NULLPTR;
}

void NysseCard::Private::readResp(NfcIsoDepClient*, const GUtilData* aResponse,
    guint aSw, const GError* aError, void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError) {
        const DataBlock* block = self->currentBlock();
        QByteArray* data = self->iData + self->iCurrentBlock;
        data->append(Util::toByteArray(aResponse));
        if (aSw == SW_OK) {
            const uint size = data->size();
            if ((block->iExpectedSize && size != block->iExpectedSize) ||
                (block->iRecordSize && (size % block->iRecordSize))) {
                HDEBUG("READ" << block->iName << "unexpected size" << size);
                self->readFailed();
            } else {
                HDEBUG("READ" << block->iName << "ok" << size << "bytes");
                self->readNextBlock();
            }
        } else if (aSw == SW_MORE) {
            HDEBUG("READ_MORE");
            nfc_isodep_client_transmit(self->iIsoDep, &READ_MORE_CMD,
                self->iCancel, readResp, self, Q_NULLPTR);
        } else {
            HDEBUG("READ" << block->iName << "unexpected status" << hex << aSw);
            self->readFailed();
        }
    } else {
        REPORT_ERROR("READ", aSw, aError);
        self->readFailed();
    }
}

void NysseCard::Private::prepareResp(NfcIsoDepClient*, const GUtilData*,
    guint aSw, const GError* aError, void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK) {
        const DataBlock* block = self->currentBlock();
        HDEBUG("PREPARE" << block->iName << "ok");
        HDEBUG("READ" << block->iName);
        nfc_isodep_client_transmit(self->iIsoDep, block->iReadApdu,
            self->iCancel, readResp, self, Q_NULLPTR);
    } else {
        REPORT_ERROR("PREPARE", aSw, aError);
        self->readFailed();
    }
}

void NysseCard::Private::readNextBlock()
{
    iCurrentBlock++;
    const DataBlock* block = currentBlock();
    if (block) {
        HDEBUG(iCurrentBlock);
        HDEBUG("PREPARE" << block->iName);
        nfc_isodep_client_transmit(iIsoDep, block->iPrepareApdu,
            iCancel, prepareResp, this, Q_NULLPTR);
    } else {
        readSucceeded();
    }
}

void NysseCard::Private::selectResp(NfcIsoDepClient*, const GUtilData*,
    guint aSw, const GError* aError, void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK) {
        HDEBUG("SELECT ok");
        self->readNextBlock();
    } else {
        REPORT_ERROR("SELECT", aSw, aError);
        self->readFailed();
    }
}

void NysseCard::Private::tagLockResp(NfcTagClient*, NfcTagClientLock* aLock,
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
    nfc_isodep_client_transmit(self->iIsoDep, &SELECT_CMD,
        self->iCancel, selectResp, self, Q_NULLPTR);
}

void NysseCard::Private::startReadingIfReady()
{
    if (iIsoDep->valid && iTag->valid) {
        if (iTag->present && iIsoDep->present && !iCancel) {
            iCurrentBlock = -1;
            for (int i = 0; i < BLOCK_COUNT; i++) {
                iData[i].clear();
            }
            iCancel = g_cancellable_new();
            nfc_tag_client_acquire_lock(iTag, TRUE, iCancel, tagLockResp,
                this, Q_NULLPTR);
        } else if (!iIsoDep->present) {
            // Not an ISO-DEP card
            readFailed();
        }
    }
}

void NysseCard::Private::isoDepEventHandler(NfcIsoDepClient*,
    NFC_ISODEP_PROPERTY, void* aPrivate)
{
    ((Private*)aPrivate)->startReadingIfReady();
}

void NysseCard::Private::tagEventHandler(NfcTagClient*,
    NFC_TAG_PROPERTY, void* aPrivate)
{
    ((Private*)aPrivate)->startReadingIfReady();
}

// ==========================================================================
// NysseCard::Desc
// ==========================================================================

TravelCardImpl* NysseCard::Private::newTravelCard(QString aPath, QObject* aParent)
{
    return new NysseCard(aPath, aParent);
}

void NysseCard::Private::registerTypes(const char* aUri, int v1, int v2)
{
    qmlRegisterType<NysseCardAppInfo>(aUri, v1, v2, "NysseCardAppInfo");
    qmlRegisterType<NysseCardBalance>(aUri, v1, v2, "NysseCardBalance");
    qmlRegisterType<NysseCardHistory>(aUri, v1, v2, "NysseCardHistory");
    qmlRegisterType<NysseCardOwnerInfo>(aUri, v1, v2, "NysseCardOwnerInfo");
    qmlRegisterType<NysseCardSeasonPass>(aUri, v1, v2, "NysseCardSeasonPass");
}

const TravelCardImpl::CardDesc NysseCard::Desc = {
    QStringLiteral("Nysse"),
    NysseCard::Private::newTravelCard,
    NysseCard::Private::registerTypes
};

// ==========================================================================
// NysseCard
// ==========================================================================

NysseCard::NysseCard(QString aPath, QObject* aParent) :
    TravelCardImpl(aParent),
    iPrivate(new Private(aPath, this))
{
}

NysseCard::~NysseCard()
{
    delete iPrivate;
}

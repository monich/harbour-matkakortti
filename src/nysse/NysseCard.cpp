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

#include "nfcdc_isodep.h"

#include "NysseCardAppInfo.h"
#include "NysseCardBalance.h"
#include "NysseCardHistory.h"
#include "NysseCardOwnerInfo.h"
#include "NysseCardTicketInfo.h"
#include "NysseCard.h"
#include "Util.h"

#include "HarbourDebug.h"
#include "HarbourUtil.h"

#if HARBOUR_DEBUG
#  define REPORT_ERROR(name,sw,err) \
    ((void)((err) ? HDEBUG(name " error" << (err)->message) : \
    HDEBUG(name " error" << hex << sw)))
#else
#  define REPORT_ERROR(name,sw,err) ((void)0)
#endif // HARBOUR_DEBUG

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

#define SELECT_RESPONSE_SLOT selectResponse
#define PREPARE_RESPONSE_SLOT prepareResponse
#define READ_RESPONSE_SLOT readResponse

class NysseCard::Private :
    public QObject
{
    Q_OBJECT

public:
    struct DataBlock {
        const char* iName;
        const char* iKey;
        uint iExpectedSize;
        uint iRecordSize;
        bool iRequired;
        const NfcIsoDepApdu* iPrepareApdu;
        const NfcIsoDepApdu* iReadApdu;
    };

    struct Response {
        uint iPrepareStatus;
        uint iReadStatus;
        QByteArray iData;

        void clear() {
            iPrepareStatus = 0;
            iReadStatus = 0;
            iData.clear();
        }
    };

    Private(NysseCard*);

    NysseCard* parentObject() const;
    void transmit(const NfcIsoDepApdu*, const char*);
    void readFailed();
    void readSucceeded();
    void readNextBlock();
    const DataBlock* currentBlock();

    static TravelCardImpl* newTravelCard(QString, QObject*);
    static void registerTypes(const char*, int, int);

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

private Q_SLOTS:
    void SELECT_RESPONSE_SLOT(const GUtilData*, uint, const GError*);
    void PREPARE_RESPONSE_SLOT(const GUtilData*, uint, const GError*);
    void READ_RESPONSE_SLOT(const GUtilData*, uint, const GError*);

public:
    Response iResp[BLOCK_COUNT];
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
        "APP_INFO", "appInfo", 32, 0, true,
        &PREPARE_APP_INFO_CMD, &READ_APP_INFO_CMD
    },{
        "OWNER_INFO", "ownerInfo", 96, 0, true,
        &PREPARE_OWNER_INFO_CMD, &READ_OWNER_INFO_CMD
    },{
        "SEASON_PASS", "ticketInfo", 96, 0, true,
        &PREPARE_SEASON_PASS_CMD, &READ_SEASON_PASS_CMD
    },{
        "HISTORY", "history", 0, 16, true,
        &PREPARE_HISTORY_CMD, &READ_HISTORY_CMD
    },{
        "BALANCE", "balance", 4, 0, false,
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

NysseCard::Private::Private(
    NysseCard* aParent) :
    QObject(aParent),
    iCurrentBlock(-1)
{}

inline
NysseCard*
NysseCard::Private::parentObject() const
{
    return qobject_cast<NysseCard*>(parent());
}

inline
void
NysseCard::Private::transmit(
    const NfcIsoDepApdu* aApdu,
    const char* aMethod)
{
    parentObject()->transmit(aApdu, this, aMethod);
}

void
NysseCard::Private::readSucceeded()
{
    QVariantMap cardInfo;
    cardInfo.insert(Util::CARD_TYPE_KEY, Desc.iName);
    for (int i = 0; i < BLOCK_COUNT; i++) {
        const Response* resp = iResp + i;
        const char* key = DATA_BLOCKS[i].iKey;
        cardInfo.insert(QString::asprintf("%sData", key),
            HarbourUtil::toHex(resp->iData)),
        cardInfo.insert(QString::asprintf("%sStatus1", key),
            QString::asprintf("%04x", resp->iPrepareStatus));
        cardInfo.insert(QString::asprintf("%sStatus2", key),
            QString::asprintf("%04x", resp->iReadStatus));
    }
    parentObject()->success(PAGE_URL, cardInfo);
}

void
NysseCard::Private::readFailed()
{
    parentObject()->failure(IoError);
}

const NysseCard::Private::DataBlock*
NysseCard::Private::currentBlock()
{
    return (iCurrentBlock >= 0 && iCurrentBlock < BLOCK_COUNT) ?
        (DATA_BLOCKS + iCurrentBlock) : Q_NULLPTR;
}

void
NysseCard::Private::readNextBlock()
{
    iCurrentBlock++;
    const DataBlock* block = currentBlock();
    if (block) {
        HDEBUG(iCurrentBlock);
        HDEBUG("PREPARE" << block->iName);
        transmit(block->iPrepareApdu, QT_STRINGIFY(PREPARE_RESPONSE_SLOT));
    } else {
        readSucceeded();
    }
}

void
NysseCard::Private::READ_RESPONSE_SLOT(
    const GUtilData* aResponse,
    uint aSw,
    const GError* aError)
{
    if (!aError) {
        const DataBlock* block = currentBlock();
        Response* resp = iResp + iCurrentBlock;

        resp->iData.append(Util::toByteArray(aResponse));
        if (aSw == SW_MORE) {
            HDEBUG("READ_MORE");
            transmit(&READ_MORE_CMD, QT_STRINGIFY(READ_RESPONSE_SLOT));
        } else {
            resp->iReadStatus = aSw;
            if (aSw == SW_OK) {
                const uint size = resp->iData.size();
                if ((block->iExpectedSize && size != block->iExpectedSize) ||
                    (block->iRecordSize && (size % block->iRecordSize))) {
                    HDEBUG("READ" << block->iName << "unexpected size" << size);
                    if (block->iRequired) {
                        readFailed();
                    } else {
                        readNextBlock();
                    }
                } else {
                    HDEBUG("READ" << block->iName << "ok" << size << "bytes");
                    readNextBlock();
                }
            } else if (!block->iRequired) {
                HDEBUG("Ignoring READ" << block->iName << "error" << hex << aSw);
                readNextBlock();
            } else {
                HDEBUG("READ" << block->iName << "unexpected status" << hex << aSw);
                readFailed();
            }
        }
    } else {
        REPORT_ERROR("READ", aSw, aError);
        readFailed();
    }
}

void
NysseCard::Private::PREPARE_RESPONSE_SLOT(
    const GUtilData*,
    uint aSw,
    const GError* aError)
{
    if (!aError) {
        const DataBlock* block = currentBlock();

        iResp[iCurrentBlock].iPrepareStatus = aSw;
        if (aSw == SW_OK) {
            HDEBUG("PREPARE" << block->iName << "ok");
            HDEBUG("READ" << block->iName);
            transmit(block->iReadApdu, QT_STRINGIFY(READ_RESPONSE_SLOT));
        } else if (!block->iRequired) {
            HDEBUG("Skipping" << block->iName << "block due to PREPARE error" << hex << aSw);
            readNextBlock();
        } else {
            HDEBUG("PREPARE" << block->iName << "unexpected status" << hex << aSw);
            readFailed();
        }
    } else {
        REPORT_ERROR("PREPARE", aSw, aError);
        readFailed();
    }
}

void
NysseCard::Private::SELECT_RESPONSE_SLOT(
    const GUtilData*,
    uint aSw,
    const GError* aError)
{
    if (!aError && aSw == SW_OK) {
        HDEBUG("SELECT ok");
        readNextBlock();
    } else {
        REPORT_ERROR("SELECT", aSw, aError);
        readFailed();
    }
}

// ==========================================================================
// NysseCard
// ==========================================================================

NysseCard::NysseCard(
    QString aPath,
    QObject* aParent) :
    TravelCardIsoDep(aPath, aParent),
    iPrivate(new Private(this))
{}

void
NysseCard::startIo()
{
    HDEBUG("SELECT");
    transmit(&Private::SELECT_CMD, iPrivate, QT_STRINGIFY(SELECT_RESPONSE_SLOT));
}

// ==========================================================================
// NysseCard::Desc
// ==========================================================================

#define REGISTER_TYPE(t,uri,v1,v2) qmlRegisterType<t>(uri,v1,v2,#t)

TravelCardImpl*
NysseCard::Private::newTravelCard(
    QString aPath,
    QObject* aParent)
{
    return new NysseCard(aPath, aParent);
}

void
NysseCard::Private::registerTypes(
    const char* aUri,
    int v1,
    int v2)
{
    REGISTER_TYPE(NysseCardAppInfo, aUri, v1, v2);
    REGISTER_TYPE(NysseCardBalance, aUri, v1, v2);
    REGISTER_TYPE(NysseCardHistory, aUri, v1, v2);
    REGISTER_TYPE(NysseCardOwnerInfo, aUri, v1, v2);
    REGISTER_TYPE(NysseCardTicketInfo, aUri, v1, v2);
}

const TravelCardImpl::CardDesc NysseCard::Desc = {
    QStringLiteral("Nysse"),
    NysseCard::Private::newTravelCard,
    NysseCard::Private::registerTypes
};

#include "NysseCard.moc"

/*
 * Copyright (C) 2019 Jolla Ltd.
 * Copyright (C) 2019 Slava Monich <slava.monich@jolla.com>
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

#include "HslCard.h"

#include "HarbourDebug.h"

enum isodep_events {
    ISODEP_EVENT_VALID,
    ISODEP_EVENT_PRESENT,
    ISODEP_EVENT_COUNT
};

// ==========================================================================
// HslCard::Private
// ==========================================================================

class HslCard::Private {
public:

    Private(HslCard* aParent);
    ~Private();

    void setPath(const char* aPath);
    void dropTag();
    void readFailed();
    void readSucceeded();
    void startReadingIfReady();
    void handleEvent(const char* aSignal);

    void emitReadingChanged();
    void emitFailedChanged();
    void emitHslTravelCard(QString aAppInfo, QString aControlInfo,
        QString aPeriodPass, QString aStoredValue, QString aTicket,
        QString aHistory);

    static QString toHexString(QByteArray aData);
    static QByteArray toByteArray(const GUtilData* aData);
    static void validChanged(NfcIsoDepClient*, NFC_ISODEP_PROPERTY, void*);
    static void presentChanged(NfcIsoDepClient*, NFC_ISODEP_PROPERTY, void*);
    static void selectResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readAppInfoResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readCtrlInfoResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readPeriodPassResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readStoredValueResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readEticketResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readHistoryResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);
    static void readMoreResp(NfcIsoDepClient*, const GUtilData*, guint, const GError*, void*);

    static const uchar SELECT_CMD_DATA[];
    static const uchar READ_APPINFO_CMD_DATA[];
    static const uchar READ_CTRLINFO_CMD_DATA[];
    static const uchar READ_PERIODPASS_CMD_DATA[];
    static const uchar READ_STOREDVALUE_CMD_DATA[];
    static const uchar READ_ETICKET_CMD_DATA[];
    static const uchar READ_HISTORY_CMD_DATA[];

    static const NfcIsoDepApdu SELECT_CMD;
    static const NfcIsoDepApdu READ_APPINFO_CMD;
    static const NfcIsoDepApdu READ_CTRLINFO_CMD;
    static const NfcIsoDepApdu READ_PERIODPASS_CMD;
    static const NfcIsoDepApdu READ_STOREDVALUE_CMD;
    static const NfcIsoDepApdu READ_ETICKET_CMD;
    static const NfcIsoDepApdu READ_HISTORY_CMD;
    static const NfcIsoDepApdu READ_MORE_CMD;

    static const uint APPINFO_SIZE = 11;
    static const uint CTRLINFO_SIZE = 10;
    static const uint PERIODPASS_SIZE = 35;
    static const uint STOREDVALUE_SIZE = 13;
    static const uint ETICKET_SIZE = 45;

    static const uint SW_OK = NFC_ISODEP_SW(0x91, 0x00);
    static const uint SW_MORE = NFC_ISODEP_SW(0x91, 0xaf);

public:
    HslCard* iParent;
    NfcIsoDepClient* iIsoDep;
    gulong iIsoDepEventId[ISODEP_EVENT_COUNT];
    GCancellable* iCancel;
    bool iReadFailed;
    QByteArray iAppInfoData;
    QByteArray iCtrlInfoData;
    QByteArray iPeriodPassData;
    QByteArray iStoredValueData;
    QByteArray iEticketData;
    QByteArray iHistoryData;
};

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

const uchar HslCard::Private::READ_CTRLINFO_CMD_DATA[] = {
    0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00
};
const NfcIsoDepApdu HslCard::Private::READ_CTRLINFO_CMD = {
    0x90, 0xbd, 0x00, 0x00,
    { READ_CTRLINFO_CMD_DATA, sizeof(READ_CTRLINFO_CMD_DATA) },
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

HslCard::Private::Private(HslCard* aParent) :
    iParent(aParent),
    iIsoDep(Q_NULLPTR),
    iCancel(Q_NULLPTR),
    iReadFailed(false)
{
    memset(iIsoDepEventId, 0, sizeof(iIsoDepEventId));
}

HslCard::Private::~Private()
{
    dropTag();
}

// Qt calls from glib callbacks better go through QMetaObject::invokeMethod
// See https://bugreports.qt.io/browse/QTBUG-18434 for details

inline void HslCard::Private::emitReadingChanged()
{
    QMetaObject::invokeMethod(iParent, "readingChanged");
}

inline void HslCard::Private::emitFailedChanged()
{
    QMetaObject::invokeMethod(iParent, "failedChanged");
}

inline void HslCard::Private::emitHslTravelCard(QString aAppInfo,
    QString aControlInfo, QString aPeriodPass, QString aStoredValue,
    QString aTicket, QString aHistory)
{
    QMetaObject::invokeMethod(iParent, "hslTravelCard",
        Q_ARG(QString, aAppInfo), Q_ARG(QString, aControlInfo),
        Q_ARG(QString, aPeriodPass), Q_ARG(QString, aStoredValue),
        Q_ARG(QString, aTicket), Q_ARG(QString, aHistory));
}

inline QByteArray HslCard::Private::toByteArray(const GUtilData* aData)
{
    return QByteArray((const char*)aData->bytes, (int)aData->size);
}

QString HslCard::Private::toHexString(QByteArray aData)
{
    static const char hex[] = "0123456789abcdef";
    const int n = aData.size();
    const uchar* data = (uchar*)aData.constData();
    char* buf = (char*)malloc(2*n + 1);
    for (int i = 0; i < n; i++) {
        const uchar b = data[i];
        buf[2*i] = hex[(b & 0xf0) >> 4];
        buf[2*i+1] = hex[b & 0x0f];
    }
    buf[2*n] = 0;
    QString str(QLatin1String(buf, 2*n));
    free(buf);
    return str;
}

void HslCard::Private::setPath(const char* aPath)
{
    dropTag();
    if (aPath) {
        iIsoDep = nfc_isodep_client_new(aPath);
        iIsoDepEventId[ISODEP_EVENT_VALID] =
            nfc_isodep_client_add_property_handler(iIsoDep,
                NFC_ISODEP_PROPERTY_VALID, validChanged, this);
        iIsoDepEventId[ISODEP_EVENT_PRESENT] =
            nfc_isodep_client_add_property_handler(iIsoDep,
                NFC_ISODEP_PROPERTY_PRESENT, presentChanged, this);
        startReadingIfReady();
    } else {
        iIsoDep = Q_NULLPTR;
    }
}

void HslCard::Private::dropTag()
{
    iReadFailed = false;
    if (iCancel) {
        g_cancellable_cancel(iCancel);
        g_object_unref(iCancel);
        iCancel = Q_NULLPTR;
    }
    if (iIsoDep) {
        nfc_isodep_client_remove_all_handlers(iIsoDep, iIsoDepEventId);
        nfc_isodep_client_unref(iIsoDep);
        iIsoDep = Q_NULLPTR;
    }
}

void HslCard::Private::readSucceeded()
{
    HASSERT(!iReadFailed);
    HDEBUG("Read done");
    GCancellable* cancel = iCancel;
    if (cancel) {
        g_cancellable_cancel(cancel);
        g_object_unref(cancel);
        iCancel = Q_NULLPTR;
    }
    // Emit hslTravelCard first
    emitHslTravelCard(toHexString(iAppInfoData), toHexString(iCtrlInfoData),
        toHexString(iPeriodPassData), toHexString(iStoredValueData),
        toHexString(iEticketData), toHexString(iHistoryData));
    if (cancel) {
        emitReadingChanged();
    }
}

void HslCard::Private::readFailed()
{
    HDEBUG("Read failed");
    HASSERT(!iReadFailed);
    iReadFailed = true;
    GCancellable* cancel = iCancel;
    if (cancel) {
        g_cancellable_cancel(cancel);
        g_object_unref(cancel);
        iCancel = Q_NULLPTR;
    }

    // Emit failedChanged first
    emitFailedChanged();
    if (cancel) {
        emitReadingChanged();
    }
}

void HslCard::Private::readMoreResp(NfcIsoDepClient*,
    const GUtilData* aResponse, guint aSw, const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK) {
        self->iHistoryData += toByteArray(aResponse);
        HDEBUG("READ_MORE" << hex << aSw << dec << aResponse->size <<
            "bytes," << self->iHistoryData.size() << "total");
        self->readSucceeded();
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("READ_MORE error" << aError->message);
        } else {
            HDEBUG("READ_MORE error" << hex << aSw);
        }
#endif // HARBOUR_DEBUG
        self->readFailed();
    }
}

void HslCard::Private::readHistoryResp(NfcIsoDepClient*,
    const GUtilData* aResponse, guint aSw, const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError) {
        self->iHistoryData = toByteArray(aResponse);
        if (aSw == SW_OK) {
            HDEBUG("READ_HISTORY ok" << aResponse->size << "bytes");
            self->readSucceeded();
        } else if (aSw == SW_MORE) {
            HDEBUG("READ_HISTORY ok" << aResponse->size << "bytes");
            HDEBUG("READ_MORE");
            nfc_isodep_client_transmit(self->iIsoDep, &READ_MORE_CMD,
                self->iCancel, readMoreResp, self, Q_NULLPTR);
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
        self->iEticketData = toByteArray(aResponse);
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
        self->iStoredValueData = toByteArray(aResponse);
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
        self->iPeriodPassData = toByteArray(aResponse);
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

void HslCard::Private::readCtrlInfoResp(NfcIsoDepClient*,
    const GUtilData* aResponse, guint aSw, const GError* aError,
    void* aPrivate)
{
    Private* self = (Private*)aPrivate;

    if (!aError && aSw == SW_OK && aResponse->size == CTRLINFO_SIZE) {
        HDEBUG("READ_CTRLINFO" << aResponse->size << "bytes");
        self->iCtrlInfoData = toByteArray(aResponse);
        HDEBUG("READ_PERIODPASS");
        nfc_isodep_client_transmit(self->iIsoDep, &READ_PERIODPASS_CMD,
            self->iCancel, readPeriodPassResp, self, Q_NULLPTR);
    } else {
#if HARBOUR_DEBUG
        if (aError) {
            HDEBUG("READ_CTRLINFO error" << aError->message);
        } else if (aSw == SW_OK) {
            HDEBUG("READ_CTRLINFO unexpected size" << aResponse->size);
        } else {
            HDEBUG("READ_CTRLINFO error" << hex << aSw);
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
        self->iAppInfoData = toByteArray(aResponse);
        HDEBUG("READ_CTRLINFO");
        nfc_isodep_client_transmit(self->iIsoDep, &READ_CTRLINFO_CMD,
            self->iCancel, readCtrlInfoResp, self, Q_NULLPTR);
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

void HslCard::Private::startReadingIfReady()
{
    if (iIsoDep->valid && iIsoDep->present && !iCancel) {
        iAppInfoData.clear();
        iCtrlInfoData.clear();
        iPeriodPassData.clear();
        iStoredValueData.clear();
        iEticketData.clear();
        iHistoryData.clear();
        iCancel = g_cancellable_new();
        iReadFailed = false;
        HDEBUG("SELECT");
        nfc_isodep_client_transmit(iIsoDep, &SELECT_CMD, iCancel, selectResp,
            this, Q_NULLPTR);
    }
}

void HslCard::Private::handleEvent(const char* aSignal)
{
    const bool wasReading = iParent->reading();
    const bool wasFailed = iParent->failed();
    startReadingIfReady();
    QMetaObject::invokeMethod(iParent, aSignal);
    if (wasReading != iParent->reading()) {
        emitReadingChanged();
    }
    if (wasFailed != iParent->failed()) {
        emitFailedChanged();
    }
}

void HslCard::Private::validChanged(NfcIsoDepClient* aIsoDep,
    NFC_ISODEP_PROPERTY, void* aPrivate)
{
    ((Private*)aPrivate)->handleEvent("validChanged");
}

void HslCard::Private::presentChanged(NfcIsoDepClient* aIsoDep,
    NFC_ISODEP_PROPERTY, void* aPrivate)
{
    ((Private*)aPrivate)->handleEvent("presentChanged");
}

// ==========================================================================
// HslCard
// ==========================================================================

HslCard::HslCard(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

HslCard::~HslCard()
{
    delete iPrivate;
}

bool HslCard::valid() const
{
    return iPrivate->iIsoDep && iPrivate->iIsoDep->valid;
}

bool HslCard::present() const
{
    return iPrivate->iIsoDep && iPrivate->iIsoDep->present;
}

bool HslCard::reading() const
{
    return iPrivate->iIsoDep && iPrivate->iCancel;
}

bool HslCard::failed() const
{
    return iPrivate->iReadFailed;
}

QString HslCard::path() const
{
    return iPrivate->iIsoDep ? QString(iPrivate->iIsoDep->path) : QString();
}

void HslCard::setPath(QString aPath)
{
    const QString currentPath(path());
    if (currentPath != aPath) {
        const bool wasValid = valid();
        const bool wasPresent = present();
        const bool wasReading = reading();
        const bool wasFailed = failed();
        HDEBUG(aPath);
        if (aPath.isEmpty()) {
            iPrivate->setPath(Q_NULLPTR);
        } else {
            QByteArray bytes(aPath.toLatin1());
            iPrivate->setPath(bytes.constData());
        }
        static bool isValid = valid();
        Q_EMIT pathChanged();
        if (wasValid && !isValid) {
            // valid has become false
            Q_EMIT validChanged();
        }
        if (wasPresent != present()) {
            Q_EMIT presentChanged();
        }
        if (wasFailed != failed()) {
            Q_EMIT failedChanged();
        }
        if (wasReading != reading()) {
            Q_EMIT readingChanged();
        }
        if (isValid && !wasValid) {
            // valid has become true
            Q_EMIT validChanged();
        }
    }
}

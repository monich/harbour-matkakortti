/*
 * Copyright (C) 2019-2020 Jolla Ltd.
 * Copyright (C) 2019-2020 Slava Monich <slava@monich.com>
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

#include "gutil_types.h"

#include "TravelCard.h"
#include "TravelCardImpl.h"

#include "HslCard.h"

#include "HarbourDebug.h"

// ==========================================================================
// TravelCard::Private
// ==========================================================================

class TravelCard::Private : public QObject {
    Q_OBJECT

public:
    static const TravelCardImpl::CardDesc * const gCardTypes[];

    Private(TravelCard* aParent);
    ~Private();

    static void deleteObjectLater(QObject* aObject);
    TravelCard* parentObject() const;
    void setPath(QString aPath);
    void tryNext();

private Q_SLOTS:
    void onReadFailed();
    void onReadDone(QString aPageUrl, QVariantMap aCardInfo);

public:
    QString iPath;
    TravelCardImpl* iCardImpl;
    int iImplIndex;
    CardState iCardState;
    QVariantMap iCardInfo;
    QString iPageUrl;
};

TravelCard::Private::Private(TravelCard* aParent) :
    QObject(aParent),
    iCardImpl(Q_NULLPTR),
    iImplIndex(-1),
    iCardState(CardNone)
{
}

const TravelCardImpl::CardDesc* const TravelCard::Private::gCardTypes[] = {
    &HslCard::Desc
};

TravelCard::Private::~Private()
{
    delete iCardImpl;
}

inline TravelCard* TravelCard::Private::parentObject() const
{
    return qobject_cast<TravelCard*>(parent());
}

void TravelCard::Private::deleteObjectLater(QObject* aObject)
{
    // See https://bugreports.qt.io/browse/QTBUG-18434
    QMetaObject::invokeMethod(aObject, "deleteLater", Qt::QueuedConnection);
}

void TravelCard::Private::setPath(QString aPath)
{
    if (iPath != aPath) {
        iPath = aPath;
        HDEBUG(aPath);
        iImplIndex = aPath.isEmpty() ? G_N_ELEMENTS(gCardTypes) : (-1);
        tryNext();
        parentObject()->pathChanged();
    }
}

void TravelCard::Private::tryNext()
{
    const CardState prevState = iCardState;
    const QVariantMap prevCardInfo(iCardInfo);
    if (iCardImpl) {
        iCardImpl->disconnect(this);
        deleteObjectLater(iCardImpl);
        iCardImpl = Q_NULLPTR;
    }
    if (++iImplIndex < (int)G_N_ELEMENTS(gCardTypes)) {
        iCardInfo.clear();
        iCardState = CardReading;
        iCardImpl = gCardTypes[iImplIndex]->iNewCard(iPath, this);
        connect(iCardImpl, SIGNAL(readFailed()), SLOT(onReadFailed()));
        connect(iCardImpl,
            SIGNAL(readDone(QString,QVariantMap)),
            SLOT(onReadDone(QString,QVariantMap)));
    } else if (iCardState == CardReading) {
        iCardState = CardNone;
        iCardInfo.clear();
    }
    if (prevState != iCardState) {
        Q_EMIT parentObject()->cardStateChanged();
    }
    if (prevCardInfo.isEmpty() != iCardInfo.isEmpty()) {
        Q_EMIT parentObject()->cardInfoChanged();
    }
}

void TravelCard::Private::onReadFailed()
{
    HDEBUG(gCardTypes[iImplIndex]->iName);
    tryNext();
}

void TravelCard::Private::onReadDone(QString aPageUrl, QVariantMap aCardInfo)
{
    HDEBUG(gCardTypes[iImplIndex]->iName << aPageUrl << aCardInfo);
    iCardImpl->disconnect(this);
    deleteObjectLater(iCardImpl);
    iCardImpl = Q_NULLPTR;
    iCardState = CardRecognized;
    iCardInfo = aCardInfo;
    iImplIndex = -1;
    TravelCard* obj = parentObject();
    if (iPageUrl != aPageUrl) {
        iPageUrl = aPageUrl;
        Q_EMIT obj->pageUrlChanged();
    }
    Q_EMIT obj->cardInfoChanged();
    Q_EMIT obj->cardStateChanged();
}

// ==========================================================================
// TravelCard
// ==========================================================================

TravelCard::TravelCard(QObject* aParent) :
    QObject(aParent),
    iPrivate(new Private(this))
{
}

void TravelCard::registerTypes(const char* aUri, int v1, int v2)
{
    for (int i = 0; i < (int)G_N_ELEMENTS(Private::gCardTypes); i++) {
        Private::gCardTypes[i]->iRegisterTypes(aUri, v1, v2);
    }
}

TravelCard::CardState TravelCard::cardState() const
{
    return iPrivate->iCardState;
}

QVariantMap TravelCard::cardInfo() const
{
    return iPrivate->iCardInfo;
}

QString TravelCard::pageUrl() const
{
    return iPrivate->iPageUrl;
}

QString TravelCard::path() const
{
    return iPrivate->iPath;
}

void TravelCard::setPath(QString aPath)
{
    iPrivate->setPath(aPath);
}

#include "TravelCard.moc"

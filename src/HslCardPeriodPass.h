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

#ifndef HSL_CARD_PERIOD_PASS_H
#define HSL_CARD_PERIOD_PASS_H

#include "HslData.h"

class HslCardPeriodPass : public HslData {
    Q_OBJECT
    Q_DISABLE_COPY(HslCardPeriodPass)
    Q_PROPERTY(QString data READ data WRITE setData NOTIFY dataChanged)
    Q_PROPERTY(HslArea validityArea READ validityArea NOTIFY validityAreaChanged)
    Q_PROPERTY(QString validityAreaName READ validityAreaName NOTIFY validityAreaChanged)
    Q_PROPERTY(QDateTime periodStartDate READ periodStartDate NOTIFY periodStartDateChanged)
    Q_PROPERTY(QDateTime periodEndDate READ periodEndDate NOTIFY periodEndDateChanged)
    Q_PROPERTY(QDateTime loadingTime READ loadingTime NOTIFY loadingTimeChanged)
    Q_PROPERTY(int loadedPeriodDays READ loadedPeriodDays NOTIFY loadedPeriodDaysChanged)
    Q_PROPERTY(int loadedPeriodPrice READ loadedPeriodPrice NOTIFY loadedPeriodPriceChanged)
    Q_PROPERTY(int daysRemaining READ daysRemaining NOTIFY daysRemainingChanged)

public:
    HslCardPeriodPass(QObject* aParent = Q_NULLPTR);
    ~HslCardPeriodPass();

    QString data() const;
    void setData(QString aData);

    HslArea validityArea() const;
    QString validityAreaName() const;
    QDateTime periodStartDate() const;
    QDateTime periodEndDate() const;
    QDateTime loadingTime() const;
    int loadedPeriodDays() const;
    int loadedPeriodPrice() const;
    int daysRemaining() const;

private Q_SLOTS:
    void updateDaysRemaining();

Q_SIGNALS:
    void dataChanged();
    void validityAreaChanged();
    void periodStartDateChanged();
    void periodEndDateChanged();
    void loadingTimeChanged();
    void loadedPeriodDaysChanged();
    void loadedPeriodPriceChanged();
    void daysRemainingChanged();

private:
    class Private;
    Private* iPrivate;
};

QML_DECLARE_TYPE(HslCardPeriodPass)

#endif // HSL_CARD_PERIOD_PASS_H

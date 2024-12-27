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

#ifndef HSL_CARD_PERIOD_PASS_H
#define HSL_CARD_PERIOD_PASS_H

#include "HslData.h"

class HslCardPeriodPass :
    public HslData
{
    Q_OBJECT
    Q_PROPERTY(QString data READ data WRITE setData NOTIFY dataChanged)
    Q_PROPERTY(int effectiveDaysRemaining READ effectiveDaysRemaining NOTIFY effectiveDaysRemainingChanged)
    Q_PROPERTY(QDateTime effectiveEndDate READ effectiveEndDate NOTIFY effectiveEndDateChanged)
    // Period 1 is either the active or the last loaded period.
    // This period is displayed by default.
    Q_PROPERTY(bool periodValid1 READ periodValid1 NOTIFY periodValid1Changed)
    Q_PROPERTY(int periodPrice1 READ periodPrice1 NOTIFY periodPrice1Changed)
    Q_PROPERTY(int periodDays1 READ periodDays1 NOTIFY periodDays1Changed)
    Q_PROPERTY(int periodDaysRemaining1 READ periodDaysRemaining1 NOTIFY periodDaysRemaining1Changed)
    Q_PROPERTY(HslArea validityArea1 READ validityArea1 NOTIFY validityArea1Changed)
    Q_PROPERTY(QString validityAreaName1 READ validityAreaName1 NOTIFY validityArea1Changed)
    Q_PROPERTY(QDateTime periodStartDate1 READ periodStartDate1 NOTIFY periodStartDate1Changed)
    Q_PROPERTY(QDateTime periodEndDate1 READ periodEndDate1 NOTIFY periodEndDate1Changed)
    Q_PROPERTY(QDateTime loadingTime1 READ loadingTime1 NOTIFY loadingTime1Changed)
    // Period 2 is the alternative period. It could be either the next
    // (not yet valid) one, the last ot the second last loaded period.
    Q_PROPERTY(bool periodValid2 READ periodValid2 NOTIFY periodValid2Changed)
    Q_PROPERTY(int periodPrice2 READ periodPrice2 NOTIFY periodPrice2Changed)
    Q_PROPERTY(int periodDays2 READ periodDays2 NOTIFY periodDays2Changed)
    Q_PROPERTY(int periodDaysRemaining2 READ periodDaysRemaining2 NOTIFY periodDaysRemaining2Changed)
    Q_PROPERTY(HslArea validityArea2 READ validityArea2 NOTIFY validityArea2Changed)
    Q_PROPERTY(QString validityAreaName2 READ validityAreaName2 NOTIFY validityArea2Changed)
    Q_PROPERTY(QDateTime periodStartDate2 READ periodStartDate2 NOTIFY periodStartDate2Changed)
    Q_PROPERTY(QDateTime periodEndDate2 READ periodEndDate2 NOTIFY periodEndDate2Changed)
    Q_PROPERTY(QDateTime loadingTime2 READ loadingTime2 NOTIFY loadingTime2Changed)
    class PeriodPass;
    class Types;

public:
    HslCardPeriodPass(QObject* aParent = Q_NULLPTR);
    ~HslCardPeriodPass();

    const QString data() const;
    void setData(const QString);

    int effectiveDaysRemaining() const;
    QDateTime effectiveEndDate() const;

    bool periodValid1() const;
    int periodPrice1() const;
    int periodDays1() const;
    int periodDaysRemaining1() const;
    HslArea validityArea1() const;
    const QString validityAreaName1() const;
    QDateTime periodStartDate1() const;
    QDateTime periodEndDate1() const;
    QDateTime loadingTime1() const;

    bool periodValid2() const;
    int periodPrice2() const;
    int periodDays2() const;
    int periodDaysRemaining2() const;
    HslArea validityArea2() const;
    const QString validityAreaName2() const;
    QDateTime periodStartDate2() const;
    QDateTime periodEndDate2() const;
    QDateTime loadingTime2() const;

Q_SIGNALS:
    void dataChanged();
    void effectiveDaysRemainingChanged();
    void effectiveEndDateChanged();
    void periodValid1Changed();
    void periodPrice1Changed();
    void periodDays1Changed();
    void periodDaysRemaining1Changed();
    void validityArea1Changed();
    void periodStartDate1Changed();
    void periodEndDate1Changed();
    void loadingTime1Changed();
    void periodValid2Changed();
    void periodPrice2Changed();
    void periodDays2Changed();
    void periodDaysRemaining2Changed();
    void validityArea2Changed();
    void periodStartDate2Changed();
    void periodEndDate2Changed();
    void loadingTime2Changed();

private:
    class Private;
    Private* iPrivate;
};

#endif // HSL_CARD_PERIOD_PASS_H

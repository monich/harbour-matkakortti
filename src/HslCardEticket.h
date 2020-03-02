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

#ifndef HSL_CARD_ETICKET_H
#define HSL_CARD_ETICKET_H

#include "HslData.h"

class HslCardEticket : public HslData {
    Q_OBJECT
    Q_DISABLE_COPY(HslCardEticket)
    Q_PROPERTY(QString data READ data WRITE setData NOTIFY dataChanged)
    Q_PROPERTY(Language language READ language NOTIFY languageChanged)
    Q_PROPERTY(ValidityLengthType validityLengthType READ validityLengthType NOTIFY validityLengthTypeChanged)
    Q_PROPERTY(int validityLength READ validityLength NOTIFY validityLengthChanged)
    Q_PROPERTY(HslArea validityArea READ validityArea NOTIFY validityAreaChanged)
    Q_PROPERTY(QString validityAreaName READ validityAreaName NOTIFY validityAreaChanged)
    Q_PROPERTY(int ticketPrice READ ticketPrice NOTIFY ticketPriceChanged)
    Q_PROPERTY(int groupSize READ groupSize NOTIFY groupSizeChanged)
    Q_PROPERTY(bool extraZone READ extraZone NOTIFY extraZoneChanged)
    Q_PROPERTY(int extensionFare READ extensionFare NOTIFY extensionFareChanged)
    Q_PROPERTY(QDateTime validityStartTime READ validityStartTime NOTIFY validityStartTimeChanged)
    Q_PROPERTY(QDateTime validityEndTime READ validityEndTime NOTIFY validityEndTimeChanged)
    Q_PROPERTY(QDateTime validityEndTimeGroup READ validityEndTimeGroup NOTIFY validityEndTimeGroupChanged)
    Q_PROPERTY(QDateTime boardingTime READ boardingTime NOTIFY boardingTimeChanged)
    Q_PROPERTY(int boardingVehicle READ boardingVehicle NOTIFY boardingVehicleChanged)
    Q_PROPERTY(HslArea boardingArea READ boardingArea NOTIFY boardingAreaChanged)
    Q_PROPERTY(QString boardingAreaName READ boardingAreaName NOTIFY boardingAreaChanged)
    Q_PROPERTY(int secondsRemaining READ secondsRemaining NOTIFY secondsRemainingChanged)

public:
    HslCardEticket(QObject* aParent = Q_NULLPTR);
    ~HslCardEticket();

    QString data() const;
    void setData(QString aData);

    Language language() const;
    ValidityLengthType validityLengthType() const;
    int validityLength() const;
    HslArea validityArea() const;
    QString validityAreaName() const;
    int ticketPrice() const;
    int groupSize() const;
    bool extraZone() const;
    int extensionFare() const;
    QDateTime validityStartTime() const;
    QDateTime validityEndTime() const;
    QDateTime validityEndTimeGroup() const;
    QDateTime boardingTime() const;
    int boardingVehicle() const;
    HslArea boardingArea() const;
    QString boardingAreaName() const;
    int secondsRemaining() const;

private Q_SLOTS:
    void updateSecondsRemaining();

Q_SIGNALS:
    void dataChanged();
    void languageChanged();
    void validityLengthTypeChanged();
    void validityLengthChanged();
    void validityAreaChanged();
    void ticketPriceChanged();
    void groupSizeChanged();
    void extraZoneChanged();
    void extensionFareChanged();
    void validityStartTimeChanged();
    void validityEndTimeChanged();
    void validityEndTimeGroupChanged();
    void boardingTimeChanged();
    void boardingVehicleChanged();
    void boardingAreaChanged();
    void secondsRemainingChanged();

private:
    class Private;
    Private* iPrivate;
};

QML_DECLARE_TYPE(HslCardEticket)

#endif // HSL_CARD_ETICKET_H

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

SilicaFlickable {
    id: details

    property alias eTicket: hslCardEticket.data
    property alias storedValue: hslCardStoredValue.data
    property alias periodPass: hslCardPeriodPass.data

    property alias moneyAmount: storedValueMoneyAmount.text
    property alias ticketSecondsRemaining: hslCardEticket.secondsRemaining
    property alias periodPassDaysRemaining: hslCardPeriodPass.daysRemaining
    property alias periodPassEndDate: hslCardPeriodPass.periodEndDate

    contentHeight: childrenRect.height

    HslCardEticket {
        id: hslCardEticket

        readonly property bool haveLastTicket: HslData.isValidPeriod(validityStartTime, validityEndTime)
    }

    HslCardStoredValue {
        id: hslCardStoredValue
    }

    HslCardPeriodPass {
        id: hslCardPeriodPass

        readonly property bool haveSeasonTicket: HslData.isValidPeriod(periodStartDate, periodEndDate)
    }

    function moneyString(value) {
        return (value/100.0).toFixed(2) + " €"
    }

    function dateString(date) {
        //return date.toLocaleDateString(Qt.locale(), Locale.NarrowFormat)
        return date.toLocaleDateString(Qt.locale(), "dd.MM.yyyy")
    }

    function timeString(date) {
        return date.toLocaleTimeString(Qt.locale(), "hh:mm")
    }

    function dateTimeString(date) {
        return dateString(date) + " " + timeString(date)
    }

    function timeUnits(unit) {
        switch (unit) {
        case HslData.ValidityLengthMinute:
            //: Time unit (abbreviated)
            //% "min"
            return qsTrId("matkakortti-time_unit_abbr-min")
        case HslData.ValidityLengthHour:
            //: Time unit (abbreviated)
            //% "h"
            return qsTrId("matkakortti-time_unit_abbr-hour")
        case HslData.ValidityLength24Hours:
            //: Time unit (abbreviated)
            //% "d"
            return qsTrId("matkakortti-time_unit_abbr-24hour")
        case HslData.ValidityLengthDay:
            //: Time unit (abbreviated)
            //% "d"
            return qsTrId("matkakortti-time_unit_abbr-day")
        }
        return unit
    }

    Column {
        width: parent.width

        Column {
            width: parent.width
            visible: hslCardPeriodPass.haveSeasonTicket

            SectionHeader {
                //: Section header
                //% "Season tickets"
                text: qsTrId("matkakortti-details-section-season_tickets")
                visible: hslCardPeriodPass.haveSeasonTicket
            }

            VerticalSpace { height: Theme.paddingLarge }

            Item {
                id: seasonTicketItem

                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                height: Math.max(seasonTicketColumn.height, seasonTicketValidity.height) + Theme.paddingLarge

                Column {
                    id: seasonTicketColumn

                    anchors {
                        left: parent.left
                        right: seasonTicketValidity.left
                        rightMargin: Theme.paddingLarge
                    }

                    ValueLabel {
                        width: parent.width
                        //: Label
                        //% "Zone:"
                        title: qsTrId("matkakortti-details-zone")
                        value: hslCardPeriodPass.validityAreaName
                    }

                    Label {
                        width: parent.width
                        horizontalAlignment: Text.AlignLeft
                        color: Theme.highlightColor
                        wrapMode: Text.Wrap
                        text: dateString(hslCardPeriodPass.periodStartDate) + " - " + dateString(hslCardPeriodPass.periodEndDate)
                    }
                }

                ValidityItem {
                    id: seasonTicketValidity

                    valid: hslCardPeriodPass.daysRemaining
                    anchors {
                        top: parent.top
                        right: parent.right
                    }
                }
            }
        }

        SectionHeader {
            //: Section header
            //% "Card value"
            text: qsTrId("matkakortti-details-section-card_value")
        }

        VerticalSpace { height: Theme.paddingLarge }

        Item {
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            height: Math.max(lastLoadColumn.height, storedValueMoneyAmount.height)

            Column {
                id: lastLoadColumn

                anchors {
                    left: parent.left
                    right: storedValueMoneyAmount.right
                    rightMargin: Theme.paddingLarge
                }

                ValueLabel {
                    width: parent.width
                    visible: hslCardStoredValue.loadedValue > 0
                    //: Label
                    //% "Last loaded value:"
                    title: qsTrId("matkakortti-details-loaded_value-label")
                    value: moneyString(hslCardStoredValue.loadedValue)
                }

                ValueLabel {
                    width: parent.width
                    visible: HslData.isValidDate(hslCardStoredValue.loadingTime)
                    //: Label
                    //% "Loading time:"
                    title: qsTrId("matkakortti-details-loading_time-label")
                    value: dateTimeString(hslCardStoredValue.loadingTime)
                }
            }

            Text {
                id: storedValueMoneyAmount

                anchors.right: parent.right
                horizontalAlignment: Text.AlignRight
                font {
                    pixelSize: Theme.fontSizeLarge
                    bold: true
                }
                color: Theme.primaryColor
                text: moneyString(hslCardStoredValue.moneyValue)
            }
        }

        VerticalSpace { height: Theme.paddingLarge }

        Column {
            width: parent.width
            visible: hslCardEticket.haveLastTicket

            SectionHeader {
                //: Section header
                //% "Latest value ticket"
                text: qsTrId("matkakortti-details-section-latest_ticket")
            }

            VerticalSpace { height: Theme.paddingLarge }

            Item {
                id: lastTicketItem

                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                height: Math.max(lastTicketColumn.height, lastTicketValidity.height)

                Column {
                    id: lastTicketColumn

                    anchors {
                        left: parent.left
                        right: lastTicketValidity.left
                        rightMargin: Theme.paddingLarge
                    }

                    ValueLabel {
                        width: parent.width
                        //: Label
                        //% "Zone:"
                        title: qsTrId("matkakortti-details-zone")
                        value: hslCardEticket.validityAreaName
                    }

                    ValueLabel {
                        width: parent.width
                        //: Label
                        //% "Group size:"
                        title: qsTrId("matkakortti-details-ticket-group_size")
                        value: hslCardEticket.groupSize
                    }

                    ValueLabel {
                        width: parent.width
                        //: Label
                        //% "Cost:"
                        title: qsTrId("matkakortti-details-ticket-cost")
                        value: moneyString(hslCardEticket.ticketPrice)
                    }

                    ValueLabel {
                        width: parent.width
                        //: Label
                        //% "Validity time:"
                        title: qsTrId("matkakortti-details-ticket-validity_length")
                        value: hslCardEticket.validityLength + " " + timeUnits(hslCardEticket.validityLengthType)
                    }
                }

                ValidityItem {
                    id: lastTicketValidity

                    valid: hslCardEticket.secondsRemaining
                    anchors {
                        top: parent.top
                        right: parent.right
                    }
                }
            }

            ValueLabel {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                //: Label
                //% "Valid from:"
                title: qsTrId("matkakortti-details-ticket-valid_from")
                value: dateTimeString(hslCardEticket.validityStartTime)
            }

            ValueLabel {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                visible: HslData.isValidDate(hslCardEticket.boardingTime) &&
                    hslCardEticket.boardingTime.getTime() !== hslCardEticket.validityStartTime.getTime()
                //: Label
                //% "Boarding time:"
                title: qsTrId("matkakortti-details-ticket-boarding_time")
                value: dateTimeString(hslCardEticket.boardingTime)
            }

            VerticalSpace { height: Theme.paddingLarge }

            ValueLabel {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                //: Label
                //% "Valid until:"
                title: qsTrId("matkakortti-details-ticket-valid_until")
                value: dateTimeString(hslCardEticket.validityEndTime)
                //: Suffix after the time ending the period
                //% " "
                suffix: qsTrId("matkakortti-details-ticket-valid_until-suffix").trim()
            }

            VerticalSpace { height: Theme.paddingLarge }
        }
    }
}

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

SilicaFlickable {
    property var eTicket
    property var storedValue
    property var periodPass

    contentHeight: column.height

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
        id: column

        width: parent.width

        Column {
            width: parent.width
            visible: periodPass.periodValid1
            spacing: Theme.paddingLarge

            SectionHeader {
                //: Section header
                //% "Season tickets"
                text: qsTrId("matkakortti-details-section-season_tickets")
            }

            MouseArea {
                id: passItems

                property bool expanded
                readonly property int rowHeight: Math.max(passItem1.implicitHeight, passItem2.implicitHeight)

                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                height: expanded ? 2 * rowHeight : rowHeight
                clip: true

                Column {
                    width: parent.width

                    HslPeriodPassItem {
                        id: passItem1

                        height: passItems.rowHeight
                        startDate: periodPass.periodStartDate1
                        endDate: periodPass.periodEndDate1
                        areaName: periodPass.validityAreaName1
                        price: periodPass.periodPrice1
                        daysRemaining: periodPass.periodDaysRemaining1
                    }

                    HslPeriodPassItem {
                        id: passItem2

                        height: passItems.rowHeight
                        startDate: periodPass.periodStartDate2
                        endDate: periodPass.periodEndDate2
                        areaName: periodPass.validityAreaName2
                        price: periodPass.periodPrice2
                        daysRemaining: periodPass.periodDaysRemaining2
                    }
                }

                Behavior on height { SmoothedAnimation { duration: 200 } }

                onClicked: expanded = !expanded
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
                    visible: storedValue.loadedValue > 0
                    //: Label
                    //% "Last loaded value:"
                    title: qsTrId("matkakortti-details-loaded_value-label")
                    value: Utils.moneyString(storedValue.loadedValue)
                }

                ValueLabel {
                    width: parent.width
                    visible: HslData.isValidDate(storedValue.loadingTime)
                    //: Label
                    //% "Loading time:"
                    title: qsTrId("matkakortti-details-loading_time-label")
                    value: Utils.dateTimeString(storedValue.loadingTime)
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
                text: Utils.moneyString(storedValue.moneyValue)
            }
        }

        VerticalSpace { height: Theme.paddingLarge }

        Column {
            width: parent.width
            visible: HslData.isValidPeriod(eTicket.validityStartTime, eTicket.validityEndTime)

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
                        value: eTicket.validityAreaName
                    }

                    ValueLabel {
                        width: parent.width
                        //: Label
                        //% "Group size:"
                        title: qsTrId("matkakortti-details-ticket-group_size")
                        value: eTicket.groupSize
                        visible: eTicket.groupSize > 1
                    }

                    ValueLabel {
                        width: parent.width
                        //: Label
                        //% "Cost:"
                        title: qsTrId("matkakortti-details-ticket-cost")
                        value: Utils.moneyString(eTicket.extraZone ? eTicket.extensionFare : eTicket.ticketPrice)
                    }

                    ValueLabel {
                        width: parent.width
                        //: Label
                        //% "Validity time:"
                        title: qsTrId("matkakortti-details-ticket-validity_length")
                        value: eTicket.validityLength + " " + timeUnits(eTicket.validityLengthType)
                    }
                }

                ValidityItem {
                    id: lastTicketValidity

                    valid: eTicket.secondsRemaining
                    anchors {
                        top: parent.top
                        right: parent.right
                    }
                }
            }

            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x

                ValueLabel {
                    width: parent.width
                    //: Label
                    //% "Valid from:"
                    title: qsTrId("matkakortti-details-ticket-valid_from")
                    value: Utils.dateTimeString(eTicket.validityStartTime)
                }

                ValueLabel {
                    width: parent.width
                    //: Label
                    //% "Valid until:"
                    title: qsTrId("matkakortti-details-ticket-valid_until")
                    value: Utils.dateTimeString(eTicket.validityEndTime)
                    //: Suffix after the time ending the period
                    //% " "
                    suffix: qsTrId("matkakortti-details-ticket-valid_until-suffix").trim()
                }

                ValueLabel {
                    width: parent.width
                    visible: HslData.isValidDate(eTicket.boardingTime) &&
                        eTicket.boardingTime.getTime() !== eTicket.validityStartTime.getTime()
                    //: Label
                    //% "Boarding time:"
                    title: qsTrId("matkakortti-details-ticket-boarding_time")
                    value: Utils.dateTimeString(eTicket.boardingTime)
                }

                ValueLabel {
                    width: parent.width
                    //: Label
                    //% "Boarding zone:"
                    title: qsTrId("matkakortti-details-boarding_zone")
                    value: eTicket.boardingAreaName
                    visible: value !== ""
                }
            }

            VerticalSpace { height: Theme.paddingLarge }
        }
    }

    VerticalScrollDecorator { }
}

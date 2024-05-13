import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

SilicaFlickable {
    property var ownerInfo
    property var balance
    property var ticketInfo

    contentHeight: column.height + Theme.paddingLarge

    Column {
        id: column

        width: parent.width

        VerticalSpace { height: Theme.paddingLarge }

        MouseArea {
            id: personalInfoArea

            property bool expanded

            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            height: expanded ? personalInfoItems.height : ownerName.height
            clip: true

            Column {
                id: personalInfoItems

                width: parent.width

                Text {
                    id: ownerName

                    width: parent.width
                    font {
                        pixelSize: Theme.fontSizeLarge
                        bold: true
                    }
                    wrapMode: Text.Wrap
                    color: Theme.secondaryHighlightColor
                    text: ownerInfo.ownerName
                }

                ValueLabel {
                    width: parent.width
                    fontSize: Theme.fontSizeSmall
                    //: Label
                    //% "Birthday:"
                    title: qsTrId("matkakortti-details-card-birthday")
                    value: Utils.dateString(ownerInfo.birthDate)
                    visible: Utils.isValidDate(ownerInfo.birthDate)

                    Behavior on opacity { FadeAnimation { duration: 200 } }
                }
            }

            Behavior on height { SmoothedAnimation { duration: 200 } }

            onClicked: expanded = !expanded
        }

        ValueLabel {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            fontSize: Theme.fontSizeSmall
            //: Label
            //% "Card issue date:"
            title: qsTrId("matkakortti-details-card-issue_date")
            value: Utils.dateString(ownerInfo.issueDate)
            visible: Utils.isValidDate(ownerInfo.issueDate)
        }

        SectionHeader {
            visible: ticketInfo.valid
            //: Section header
            //% "Season tickets"
            text: qsTrId("matkakortti-details-section-season_tickets")
        }

        Item {
            visible: ticketInfo.valid
            x: Theme.horizontalPageMargin
            width: parent.width - 2*x
            height: Math.max(seasonTicketLabel.height, seasonTicketValidity.height)

            ValueLabel {
                id: seasonTicketLabel

                anchors {
                    top: parent.top
                    left: parent.left
                    right: seasonTicketValidity.left
                    rightMargin: Theme.paddingLarge
                }
                //: Label
                //% "Valid until:"
                title: qsTrId("matkakortti-details-ticket-valid_until")
                value: Utils.dateString(ticketInfo.endDate)
                //: Suffix after the time ending the period
                //% " "
                suffix: qsTrId("matkakortti-details-ticket-valid_until-suffix").trim()
            }

            ValidityItem {
                id: seasonTicketValidity

                valid: ticketInfo.daysRemaining
                anchors {
                    top: parent.top
                    right: parent.right
                }
            }
        }

        SectionHeader {
            //: Section header
            //% "Card value"
            text: qsTrId("matkakortti-details-section-card_value")
            visible: balance.valid
        }

        Text {
            id: balanceText

            anchors {
                right: parent.right
                rightMargin: Theme.horizontalPageMargin
            }
            font {
                pixelSize: Theme.fontSizeHuge
                bold: true
            }
            visible: balance.valid
            color: Theme.primaryColor
            text: Utils.moneyString(balance.balance)
        }
    }

    VerticalScrollDecorator { }
}

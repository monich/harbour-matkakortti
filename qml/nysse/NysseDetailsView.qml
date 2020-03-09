import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

SilicaFlickable {
    property var ownerInfo
    property var balance
    property var seasonPass

    contentHeight: column.height

    Column {
        id: column

        width: parent.width

        VerticalSpace { height: Theme.paddingLarge }

        Text {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            wrapMode: Text.Wrap
            color: Theme.secondaryHighlightColor
            font {
                pixelSize: Theme.fontSizeLarge
                bold: true
            }
            text: ownerInfo.ownerName
        }

        Item {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            height: balanceText.height

            readonly property real spacing: Theme.paddingMedium

            Label {
                id: balanceTextLabel

                anchors.baseline: balanceText.baseline
                width: Math.min(implicitWidth, parent.width - balanceText.width - parent.spacing)
                color: Theme.secondaryHighlightColor
                horizontalAlignment: Text.AlignLeft
                truncationMode: TruncationMode.Fade
                //: Label (available amount of money)
                //% "Balance:"
                text: qsTrId("matkakortti-details-ticket-balance")
            }

            Text {
                id: balanceText

                anchors {
                    left: balanceTextLabel.right
                    leftMargin: parent.spacing
                }
                color: Theme.primaryColor
                text: Utils.moneyString(balance.balance)
                font {
                    pixelSize: Theme.fontSizeLarge
                    bold: true
                }
            }
        }

        Column {
            width: parent.width
            visible: seasonPass.valid

            SectionHeader {
                //: Section header
                //% "Season tickets"
                text: qsTrId("matkakortti-details-section-season_tickets")
            }

            Item {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*x
                height: Math.max(seasonTicketLabel.height, seasonTicketValidity.height) + Theme.paddingLarge

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
                    value: Utils.dateString(seasonPass.endDate)
                    //: Suffix after the time ending the period
                    //% " "
                    suffix: qsTrId("matkakortti-details-ticket-valid_until-suffix").trim()
                }

                ValidityItem {
                    id: seasonTicketValidity

                    valid: seasonPass.daysRemaining
                    anchors {
                        top: parent.top
                        right: parent.right
                    }
                }
            }
        }
    }

    VerticalScrollDecorator { }
}

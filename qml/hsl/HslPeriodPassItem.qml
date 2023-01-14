import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

Item {
    property date startDate
    property date endDate
    property string areaName
    property int price
    property int daysRemaining

    width: parent.width
    implicitHeight: Math.max(infoColumn.height, validityItem.height) + Theme.paddingLarge

    Column {
        id: infoColumn

        anchors {
            left: parent.left
            right: validityItem.left
            rightMargin: Theme.paddingLarge
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignLeft
            color: Theme.highlightColor
            wrapMode: Text.Wrap
            text: HslData.finnishDateString(startDate) + " - " + HslData.finnishDateString(endDate)
        }

        ValueLabel {
            width: parent.width
            //: Label
            //% "Zone:"
            title: qsTrId("matkakortti-details-zone")
            value: areaName
        }

        ValueLabel {
            width: parent.width
            visible: price > 0
            //: Label
            //% "Cost:"
            title: qsTrId("matkakortti-details-ticket-cost")
            value: Utils.moneyString(price)
        }
    }

    ValidityItem {
        id: validityItem

        valid: daysRemaining
        anchors {
            top: parent.top
            right: parent.right
        }
    }
}

import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

BackgroundItem {
    property int type
    property alias time: timestamp.value
    property int moneyAmount

    height: Math.max(implicitHeight, column.height + Theme.paddingLarge)

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: parent.width - 2*x
        anchors.verticalCenter: parent.verticalCenter

        ValueLabel {
            id: timestamp

            width: parent.width
            //: Label (generic timestamp, date + time)
            //% "Time:"
            title: qsTrId("matkakortti-history-time")
        }

        ValueLabel {
            width: Math.min(preferredWidth, parent.width)
            //: Label
            //% "Cost:"
            title: qsTrId("matkakortti-details-ticket-cost")
            value: Utils.moneyString(moneyAmount)
            visible: moneyAmount > 0
        }
    }
}

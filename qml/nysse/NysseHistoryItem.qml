import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

BackgroundItem {
    property int type
    property alias time: timestamp.value
    property int moneyAmount
    property alias separator: bottomSeparator.visible

    readonly property bool _isDeposit: type === NysseCardHistory.TransactionDeposit

    height: column.y + column.height

    Column {
        id: column

        x: Theme.horizontalPageMargin
        y: Theme.paddingLarge/2
        width: parent.width - 2*x

        ValueLabel {
            id: timestamp

            width: parent.width
            //: Label (generic timestamp, date + time)
            //% "Time:"
            title: qsTrId("matkakortti-history-time")
        }

        ValueLabel {
            width: Math.min(preferredWidth, parent.width)
            title: _isDeposit ?
                //: Label
                //% "Deposit:"
                qsTrId("matkakortti-details-deposit-amount") :
                //: Label
                //% "Cost:"
                qsTrId("matkakortti-details-ticket-cost")
            value: Utils.moneyString(moneyAmount)
            boldValue: _isDeposit
            visible: moneyAmount > 0
        }

        VerticalSpace { height: column.y }

        ListSeparator { id: bottomSeparator }
    }
}

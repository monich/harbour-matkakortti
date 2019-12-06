import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

BackgroundItem {
    property int type
    property date time
    property int price
    property int group

    height: column.height + Theme.paddingLarge

    function moneyString(value) {
        return value ? ((value/100.0).toFixed(2) + " €") : ""
    }

    Column {
        id: column

        x: Theme.horizontalPageMargin
        width: parent.width - 2*x
        anchors.verticalCenter: parent.verticalCenter

        Label {
            width: parent.width
            color: Theme.secondaryHighlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
            font.bold: true
            text: time.toLocaleDateString(Qt.locale())
        }

        Label {
            width: parent.width
            color: Theme.secondaryHighlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
            font.bold: true
            text: time.toLocaleTimeString(Qt.locale(), "hh:mm")
        }

        Label {
            visible: type === HslCardHistory.TransactionBoarding
            width: parent.width
            color: Theme.secondaryHighlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
            //: Label (transaction type)
            //% "Season ticket or boarding"
            text: qsTrId("matkakortti-history-transaction_type")
        }

        Row {
            width: parent.width
            spacing: Theme.paddingMedium
            visible: type === HslCardHistory.TransactionPurchase

            ValueLabel {
                width: Math.min(preferredWidth, parent.width)
                //: Label
                //% "Cost:"
                title: qsTrId("matkakortti-details-ticket-cost")
                value: moneyString(price)
            }

            Label {
                visible: group > 1
                color: Theme.secondaryHighlightColor
                text: "(" + group + ")"
            }
        }
    }
}

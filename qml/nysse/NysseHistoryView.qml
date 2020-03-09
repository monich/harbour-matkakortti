import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components/Utils.js" as Utils

SilicaListView {
    delegate: NysseHistoryItem {
        width: parent.width
        type: transactionType
        time: Utils.dateTimeString(transactionTime)
        moneyAmount: model.moneyAmount
    }
    VerticalScrollDecorator { }
}

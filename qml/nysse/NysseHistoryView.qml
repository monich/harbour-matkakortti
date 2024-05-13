import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components/Utils.js" as Utils

SilicaListView {
    id: view

    delegate: NysseHistoryItem {
        width: parent.width
        type: model.transactionType
        time: Utils.dateTimeString(model.transactionTime)
        group: model.passengerCount
        moneyAmount: model.moneyAmount
        separator: (model.index + 1) < view.count
    }

    VerticalScrollDecorator { }
}

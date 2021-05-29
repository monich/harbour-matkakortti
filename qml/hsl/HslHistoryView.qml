import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

SilicaListView {
    id: view

    header: Column {
        width: parent.width

        SectionHeader {
            //: Section header
            //% "Previous journeys"
            text: qsTrId("matkakortti-history-section-previous_journeys")
        }

        VerticalSpace { height: Theme.paddingLarge/2 }
    }

    delegate: HslHistoryItem {
        width: parent.width
        type: transactionType
        time: Utils.dateTimeString(boardingTime)
        price: ticketPrice
        group: groupSize
        saldo: remainingValue
        separator: (index + 1) < view.count
    }

    VerticalScrollDecorator { }
}

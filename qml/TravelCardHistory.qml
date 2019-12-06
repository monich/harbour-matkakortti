import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

SilicaListView {
    header: Column {
        width: parent.width

        SectionHeader {
            //: Section header
            //% "Previous journeys"
            text: qsTrId("matkakortti-history-section-previous_journeys")
        }

        VerticalSpace { height: Theme.paddingLarge/2 }
    }

    delegate: HistoryItem {
        width: parent.width
        type: transactionType
        time: boardingTime
        price: ticketPrice
        group: groupSize
    }

    VerticalScrollDecorator { }
}

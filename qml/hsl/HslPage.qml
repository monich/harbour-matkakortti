import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

Page {
    id: thisPage

    property var cardInfo
    property alias cardImageUrl: header.cardImageUrl
    readonly property string remainingBalance: Utils.moneyString(storedValueParser.moneyValue)
    property alias ticketSecondsRemaining: eTicketParser.secondsRemaining
    property alias periodPassDaysRemaining: periodPassParser.effectiveDaysRemaining
    property alias periodPassEndDate: periodPassParser.effectiveEndDate

    readonly property var _debug: cardInfo ? cardInfo.debug : undefined

    showNavigationIndicator: false

    onStatusChanged: {
        if (status === PageStatus.Active) {
            showNavigationIndicator = true
        }
    }

    Binding {
        target: thisPage
        property: "backNavigation"
        value: !NfcAdapter.targetPresent
        when: showNavigationIndicator
    }

    HslCardAppInfo { id: appInfoParser; data: cardInfo.appInfo }
    HslCardEticket { id: eTicketParser; data: cardInfo.eTicket }
    HslCardStoredValue { id: storedValueParser; data: cardInfo.storedValue }
    HslCardPeriodPass { id: periodPassParser; data: cardInfo.periodPass }
    HslCardHistory { id: historyParser; data: cardInfo.history }

    TravelCardHeader {
        id: header

        cardType: cardInfo.cardType
        description: appInfoParser.cardNumber
        cardImageUrl: Qt.resolvedUrl("images/hsl-card.svg")
        onMultiClick: {
            if (_debug) {
                pageStack.push(Qt.resolvedUrl("../DebugPage.qml"),
                    { debug: Qt.binding(function() { return _debug}) })
            }
        }
    }

    ListSwitcher {
        id: switcher

        anchors {
            top: header.bottom
            topMargin: Theme.paddingLarge
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }

        //: Switcher label
        //% "Details"
        leftLabel: qsTrId("matkakortti-switcher-details")
        //: Switcher label
        //% "History"
        rightLabel: qsTrId("matkakortti-switcher-history")
        list: scroller
    }

    SilicaListView {
        id: scroller

        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        flickDeceleration: maximumFlickVelocity
        interactive: !switcher.animating
        clip: true

        readonly property real maxContentX: originX + Math.max(0, contentWidth - width)

        anchors {
            top: switcher.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        model: [ detailsViewComponent, historyViewComponent ]
        delegate: Loader {
            width: scroller.width
            height: scroller.height
            sourceComponent: modelData
        }
    }

    Component {
        id: detailsViewComponent

        HslDetailsView {
            anchors.fill: parent
            eTicket: eTicketParser
            storedValue: storedValueParser
            periodPass: periodPassParser
        }
    }

    Component {
        id: historyViewComponent

        HslHistoryView {
            anchors.fill: parent
            model: historyParser
        }
    }
}

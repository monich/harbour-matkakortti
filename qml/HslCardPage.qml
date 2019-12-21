import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

Page {
    property var cardInfo
    property alias cardImageUrl: header.cardImageUrl

    property string  moneyAmount: moneyString(storedValueParser.moneyValue)
    property alias ticketSecondsRemaining: eTicketParser.secondsRemaining
    property alias periodPassDaysRemaining: periodPassParser.daysRemaining
    property alias periodPassEndDate: periodPassParser.periodEndDate

    showNavigationIndicator: false

    onStatusChanged: {
        if (status === PageStatus.Active) {
            backNavigation = !NfcAdapter.targetPresent
            showNavigationIndicator = true
        }
    }

    HslCardAppInfo { id: appInfoParser; data: cardInfo.appInfo }
    HslCardEticket { id: eTicketParser; data: cardInfo.eTicket }
    HslCardStoredValue { id: storedValueParser; data: cardInfo.storedValue }
    HslCardPeriodPass { id: periodPassParser; data: cardInfo.periodPass }
    HslCardHistory { id: historyParser; data: cardInfo.history }

    function moneyString(value) {
        return (value/100.0).toFixed(2) + " €"
    }

    TravelCardHeader {
        id: header

        description: appInfoParser.cardNumber
        cardImageUrl: Qt.resolvedUrl("images/hsl-card.svg")
    }

    Item {
        id: switcher

        anchors {
            top: header.bottom
            topMargin: Theme.paddingLarge
            left: parent.left
            leftMargin: Theme.horizontalPageMargin
            right: parent.right
            rightMargin: Theme.horizontalPageMargin
        }

        height: highlighter.height

        readonly property real maxButtonWidth: width - Theme.paddingLarge
        readonly property real buttonWidth: Math.min(maxButtonWidth,
                Math.max(detailsButton.implicitWidth, historyButton.implicitWidth) + 2 * Theme.paddingSmall)

        Rectangle {
            id: highlighter

            readonly property real padding: Theme.paddingMedium
            x: detailsButton.x - padding + (historyButton.x -  detailsButton.x) * scroller.contentX / scroller.width
            anchors.verticalCenter: parent.verticalCenter
            radius: Theme.paddingMedium
            width: switcher.buttonWidth + 2 * padding
            height: Math.max(detailsButton.height, historyButton.height)
            color: Theme.primaryColor
            opacity: HarbourTheme.opacityFaint
        }

        PressableLabel {
            id: detailsButton

            anchors {
                verticalCenter: parent.verticalCenter
                right: historyButton.left
                rightMargin: Theme.paddingLarge
            }
            width: switcher.buttonWidth
            //: Switcher label
            //% "Details"
            text: qsTrId("matkakortti-switcher-details")
            highlighted: scroller.currentIndex === 0
            onClicked: { // Animate positionViewAtBeginning()
                if (!scrollAnimation.running && scroller.contentX > 0) {
                    scrollAnimation.from = scroller.contentX
                    scrollAnimation.to = 0
                    scrollAnimation.start()
                }
            }
        }

        PressableLabel {
            id: historyButton

            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
            }
            width: switcher.buttonWidth
            //: Switcher label
            //% "History"
            text: qsTrId("matkakortti-switcher-history")
            highlighted: scroller.currentIndex === 1
            onClicked: { // Animate positionViewAtEnd()
                if (!scrollAnimation.running && scroller.contentX < scroller.maxContentX) {
                    scrollAnimation.from = scroller.contentX
                    scrollAnimation.to = scroller.maxContentX
                    scrollAnimation.start()
                }
            }
        }

        NumberAnimation {
            id: scrollAnimation

            target: scroller
            property: "contentX"
            duration: 200
            easing.type: Easing.InOutQuad
        }
    }

    SilicaListView {
        id: scroller

        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        flickDeceleration: maximumFlickVelocity
        interactive: !scrollAnimation.running
        clip: true

        readonly property real maxContentX: Math.max(0, contentWidth - width)

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

        TravelCardDetails {
            anchors.fill: parent
            eTicket: eTicketParser
            storedValue: storedValueParser
            periodPass: periodPassParser
        }
    }

    Component {
        id: historyViewComponent

        TravelCardHistory {
            anchors.fill: parent
            model: historyParser
        }
    }
}

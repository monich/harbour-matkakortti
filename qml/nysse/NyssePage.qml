import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

import "../components"
import "../components/Utils.js" as Utils

Page {
    property var cardInfo
    property alias cardImageUrl: header.cardImageUrl
    readonly property string remainingBalance: Utils.moneyString(balanceParser.balance)
    readonly property int ticketSecondsRemaining: 0
    readonly property int periodPassDaysRemaining: 0

    showNavigationIndicator: false

    onStatusChanged: {
        if (status === PageStatus.Active) {
            backNavigation = !NfcAdapter.targetPresent
            showNavigationIndicator = true
        }
    }

    NysseCardAppInfo { id: appInfoParser; data: cardInfo.appInfo }
    NysseCardOwnerInfo { id: ownerInfoParser; data: cardInfo.ownerInfo }
    NysseCardBalance { id: balanceParser; data: cardInfo.balance }
    NysseCardHistory { id: historyParser; data: cardInfo.history }
    NysseCardSeasonPass { id: seasonPassParser; data: cardInfo.seasonPass }

    TravelCardHeader {
        id: header

        cardType: cardInfo.cardType
        description: appInfoParser.cardNumber
        cardImageUrl: Qt.resolvedUrl("images/nysse-card.svg")
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
            x: detailsButton.x - padding + (historyButton.x -  detailsButton.x) * (scroller.contentX - scroller.originX) / scroller.width
            anchors.verticalCenter: parent.verticalCenter
            radius: Theme.paddingMedium
            width: switcher.buttonWidth + 2 * padding
            height: Math.max(detailsButton.height, historyButton.height)
            color: Theme.primaryColor
            opacity: 0.2 // opacityFaint
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
                if (!scrollAnimation.running && scroller.contentX > scroller.originX) {
                    scrollAnimation.from = scroller.contentX
                    scrollAnimation.to = scroller.originX
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

        readonly property real maxContentX: scroller.originX + Math.max(0, contentWidth - width)

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

        NysseDetailsView {
            anchors.fill: parent
            ownerInfo: ownerInfoParser
            balance: balanceParser
            seasonPass: seasonPassParser
        }
    }

    Component {
        id: historyViewComponent

        NysseHistoryView {
            anchors.fill: parent
            model: historyParser
        }
    }
}

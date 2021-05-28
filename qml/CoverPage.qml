import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0
import harbour.matkakortti 1.0

import "components/Utils.js" as Matkakortti
import "harbour"

CoverBackground {
    id: cover

    property Page cardInfoPage
    property bool unrecorgnizedCard
    readonly property real extraSize: Theme.paddingMedium
    readonly property int ticketSecondsRemaining: cardInfoPage ? cardInfoPage.ticketSecondsRemaining : TravelCard.PeriodInvalid
    readonly property int periodPassDaysRemaining: cardInfoPage ? cardInfoPage.periodPassDaysRemaining : TravelCard.PeriodInvalid
    readonly property var periodPassEndDate: cardInfoPage ? cardInfoPage.periodPassEndDate : undefined
    readonly property string remainingBalance: (cardInfoPage && cardInfoPage.remainingBalance) ?
        cardInfoPage.remainingBalance : ""

    signal popCardInfo()

    function dateString(date) {
        return date ? date.toLocaleDateString(Qt.locale(), "dd.MM.yyyy") : ""
    }

    function int2(i) {
        return i < 10 ? ("0" + i) : i
    }

    ConfigurationValue {
        id: lastCardType

        key: Matkakortti.configLastCardType
    }

    function timeRemainingString(secs) {
        if (secs >= 3600) {
            var h = Math.floor(secs / 3600)
            var m = Math.floor((secs % 3600) / 60)
            return h + ":" + int2(m) + ":" + int2(secs % 60)
        } else if (secs >= 60) {
            var min = Math.floor(secs / 60)
            return min + ":" + int2(secs % 60)
        } else {
            return "0:" + int2(secs % 60)
        }
    }

    HarbourHighlightIcon {
        anchors.centerIn: parent
        sourceSize: Qt.size(parent.width + extraSize, parent.height + extraSize)
        source: (lastCardType.value === "Nysse") ? "nysse/images/nysse-cover.svg" : "hsl/images/hsl-cover.svg"
        smooth: true
        fillMode: Image.PreserveAspectCrop
        highlightColor: Theme.primaryColor
        opacity: unrecorgnizedCard ? 0 : 1
        visible: opacity > 0
        Behavior on opacity { FadeAnimation {} }
    }

    HarbourHighlightIcon {
        anchors.centerIn: parent
        width: Math.floor(3 * parent.width / 4)
        height: width
        sourceSize: Qt.size(width, height)
        source: unrecorgnizedCard ? "images/hmm.svg" : ""
        smooth: true
        opacity: unrecorgnizedCard ? 1 : 0
        visible: opacity > 0
        Behavior on opacity { FadeAnimation {} }
    }

    Image {
        anchors.centerIn: parent
        sourceSize: Qt.size(cover.height + extraSize, cover.width + extraSize)
        source: cardInfoPage ? cardInfoPage.cardImageUrl : ""
        smooth: true
        rotation: 90
        transformOrigin: Item.Center
        fillMode: Image.PreserveAspectCrop
        visible: !!cardInfoPage
    }

    Item {
        visible: ticketSecondsRemaining  > 0 || periodPassDaysRemaining > 0 || remainingBalance.length > 0
        width: remainingBalanceBackground.width
        height: remainingBalanceBackground.height
        anchors.centerIn: parent

        Rectangle {
            id: remainingBalanceBackground
            readonly property real maxWidth: cover.width - 2 * Theme.paddingMedium

            border {
                color: Theme.rgba(Theme.primaryColor, HarbourTheme.opacityLow)
                width: Theme.paddingSmall
            }
            width: Math.min(maxWidth, Math.floor(remainingBalanceLabel.width + 2 * height/3))
            height: Theme.itemSizeSmall
            radius: height/2
            color: Theme.rgba(HarbourTheme.invertedPrimaryColor, HarbourTheme.opacityOverlay)
        }

        Text {
            id: remainingBalanceLabel

            font {
                pixelSize: Theme.fontSizeLarge
                bold: true
            }
            anchors.centerIn: parent
            color: Theme.primaryColor
            text: ticketSecondsRemaining > 0 ? timeRemainingString(ticketSecondsRemaining) :
                periodPassDaysRemaining > 0 ? dateString(periodPassEndDate) : remainingBalance
        }
    }

    CoverActionList {
        enabled: !!cardInfoPage
        CoverAction {
            iconSource: "image://theme/icon-cover-cancel"
            onTriggered: cover.popCardInfo()
        }
    }
}

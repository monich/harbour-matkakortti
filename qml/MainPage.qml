import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0
import harbour.matkakortti 1.0

import "components/Utils.js" as Matkakortti
import "components"
import "harbour"

Page {
    id: page

    backNavigation: false
    showNavigationIndicator: false

    readonly property bool targetPresent: NfcAdapter.targetPresent
    readonly property bool unrecorgnizedCard: targetPresent && travelCard.cardState === TravelCard.CardNone && !readTimer.running
    readonly property bool readingCard: travelCard.cardState === TravelCard.CardReading || readTimer.running
    readonly property bool nysseSupported: NfcSystem.version >= NfcSystem.Version_1_0_26
    property bool showingCardInfo: !!cardInfoPage
    property Page cardInfoPage

    NfcTag {
        path: NfcAdapter.tagPath
        onValidChanged: maybePopCardInfoPage()
        onPresentChanged: maybePopCardInfoPage()
        onTypeChanged: maybePopCardInfoPage()
        function maybePopCardInfoPage() {
            if (valid && present && type != NfcTag.IsoDep && showingCardInfo) {
                cardInfoPage.backNavigation = true
                pageStack.pop(page, PageStackAction.Animated)
            }
        }
    }

    ConfigurationValue {
        id: lastCardType

        key: Matkakortti.configLastCardType
    }

    TravelCard {
        id: travelCard

        path: NfcAdapter.tagPath
        defaultCardType: lastCardType.value
        onCardStateChanged: {
            switch (cardState) {
            case TravelCard.CardReading:
                readTimer.start()
                break

            case TravelCard.CardRecognized:
                lastCardType.value = cardInfo.cardType
                if (cardInfoPage) {
                    if (cardInfoPage.cardInfo.cardType === cardInfo.cardType) {
                        cardInfoPage.cardInfo = cardInfo
                    } else {
                        cardInfoPage = pageStack.replace(Qt.resolvedUrl(pageUrl),
                            { cardInfo: cardInfo })
                        if (cardInfoPage) {
                            cardInfoPage.statusChanged.connect(function() {
                                if (cardInfoPage.status === PageStatus.Inactive) {
                                    cardInfoPage.destroy()
                                    cardInfoPage = null
                                }
                            })
                        }
                    }
                } else {
                    cardInfoPage = pageStack.push(Qt.resolvedUrl(pageUrl),
                        { cardInfo: cardInfo })
                    if (cardInfoPage) {
                        cardInfoPage.statusChanged.connect(function() {
                            if (cardInfoPage.status === PageStatus.Inactive) {
                                cardInfoPage.destroy()
                                cardInfoPage = null
                            }
                        })
                    }
                }
                break

            case TravelCard.CardNone:
                if (targetPresent && cardInfoPage) {
                    // Unsupported card
                    cardInfoPage.backNavigation = true
                    pageStack.pop(page, PageStackAction.Animated)
                }
                break
            }
        }
    }

    Timer {
        id: readTimer

        interval: 500
    }

    onTargetPresentChanged: {
        if (cardInfoPage && cardInfoPage.status === PageStatus.Active) {
            cardInfoPage.backNavigation = !targetPresent
        }
    }

    Item {
        anchors.fill: parent
        opacity: (NfcSystem.valid && (!NfcSystem.present || !NfcAdapter.present)) ? 1 : 0
        visible: opacity > 0
        Behavior on opacity { FadeAnimation {} }

        HarbourHighlightIcon {
            y: (parent.height/2 - height)/2
            anchors.horizontalCenter: parent.horizontalCenter
            height: page.width/2
            sourceSize.height: height
            source: parent.visible ? "images/hmm.svg" : ""
            smooth: true
        }

        InfoLabel {
            y: parent.height/2
            //: Info label
            //% "NFC not supported"
            text: qsTrId("matkakortti-info-nfc_not_supported")
            visible: !NfcSystem.present
        }
    }

    Item {
        anchors.fill: parent
        visible: (NfcSystem.valid && NfcSystem.present && NfcAdapter.present)

        ShaderEffectSource {
            width: cardImages.width
            height: cardImages.height
            opacity: (!readingCard && !targetPresent) ? 1 : 0
            visible: opacity > 0
            sourceItem: Item {
                id: cardImages

                width: page.width
                height: page.height/2

                property real cardImageHeight: Math.round(2*width/5)

                CardImageWithShadow {
                    anchors.centerIn: parent
                    sourceSize.height: parent.cardImageHeight
                    source: Qt.resolvedUrl("nysse/images/nysse-card.svg")
                    rotation: 105
                    visible: nysseSupported
                    z: (lastCardType.value === "Nysse") ? 1 : 0
                }

                CardImageWithShadow {
                    id: hslImage

                    anchors.centerIn: parent
                    sourceSize.height: parent.cardImageHeight
                    source: Qt.resolvedUrl("hsl/images/hsl-card.svg")
                    rotation: nysseSupported ? 60 : 90
                    z: (lastCardType.value === "HSL") ? 1 : 0
                }
            }
            Behavior on opacity { FadeAnimation {} }
        }

        BusyIndicator {
            y: (parent.height/2 - height)/2
            anchors.horizontalCenter: parent.horizontalCenter
            size: BusyIndicatorSize.Large
            running: true
            opacity: readingCard ? 1 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }

        HarbourHighlightIcon {
            y: (parent.height/2 - height)/2
            anchors.horizontalCenter: parent.horizontalCenter
            height: page.width/2
            sourceSize.height: height
            source: visible ? "images/hmm.svg" : ""
            smooth: true
            opacity: (!readingCard && unrecorgnizedCard) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }

        InfoLabel {
            id: statusLabel

            y: parent.height/2

            text: readingCard ?
                //: Info label
                //% "Reading the card"
                qsTrId("matkakortti-info-reading") :
                targetPresent ? (
                travelCard.cardState === TravelCard.CardNone ?
                    //: Info label
                    //% "This is not a supported travel card"
                    qsTrId("matkakortti-info-card_not_supported") : "") :
                (NfcSystem.enabled ? "" :
                    //: Info label
                    //% "NFC is disabled"
                    qsTrId("matkakortti-info-disabled"))
        }

        Text {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            height: Math.floor(parent.height/2)
            anchors {
                top: parent.top
                topMargin: cardImages.height - Theme.itemSizeMedium
                bottom: parent.bottom
            }
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            font {
                pixelSize: Theme.fontSizeLarge
                family: Theme.fontFamilyHeading
            }
            color: Theme.highlightColor
            //: Hint label
            //% "Place the phone on the card"
            text: qsTrId("matkakortti-info-touch_hint")
            opacity: (NfcSystem.enabled && !targetPresent && !readingCard) ? HarbourTheme.opacityHigh : 0
            visible: opacity > 0
            Behavior on opacity {
                enabled: NfcSystem.valid
                FadeAnimation {}
            }
        }
    }
}

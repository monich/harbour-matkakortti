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

    readonly property bool unrecorgnizedCard: NfcAdapter.targetPresent && travelCard.cardState === TravelCard.CardNone && !readTimer.running
    readonly property Page cardInfoPage: pageStack.nextPage(page)

    readonly property bool _nysseSupported: NfcSystem.version >= NfcSystem.Version_1_0_26
    readonly property bool _readingCard: travelCard.cardState === TravelCard.CardReading || readTimer.running

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
                        cardInfoPage.cardInfo = cardInfo // Reuse the existing page
                    } else {
                        pageStack.replaceAbove(page, Qt.resolvedUrl(pageUrl), { cardInfo: cardInfo })
                    }
                } else {
                    pageStack.push(Qt.resolvedUrl(pageUrl), { cardInfo: cardInfo })
                }
                break
            }
        }
    }

    Timer {
        id: readTimer

        interval: 500
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
            opacity: (!_readingCard && !NfcAdapter.targetPresent) ? 1 : 0
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
                    visible: _nysseSupported
                    z: (lastCardType.value === "Nysse") ? 1 : 0
                }

                CardImageWithShadow {
                    id: hslImage

                    anchors.centerIn: parent
                    sourceSize.height: parent.cardImageHeight
                    source: Qt.resolvedUrl("hsl/images/hsl-card.svg")
                    rotation: _nysseSupported ? 60 : 90
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
            opacity: _readingCard ? 1 : 0
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
            opacity: (!_readingCard && unrecorgnizedCard) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity { FadeAnimation {} }
        }

        InfoLabel {
            id: statusLabel

            y: parent.height/2

            text: _readingCard ?
                //: Info label
                //% "Reading the card"
                qsTrId("matkakortti-info-reading") :
                NfcAdapter.targetPresent ? (
                travelCard.cardState === TravelCard.CardNone ?
                    //: Info label
                    //% "This is not a supported travel card"
                    qsTrId("matkakortti-info-card_not_supported") : "") :
                (NfcSystem.enabled ? "" :
                    //: Info label
                    //% "NFC is off"
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
            opacity: (NfcSystem.enabled && !NfcAdapter.targetPresent && !_readingCard) ? 0.6 /* opacityHigh */ : 0
            visible: opacity > 0
            Behavior on opacity {
                enabled: NfcSystem.valid
                FadeAnimation {}
            }
        }
    }
}

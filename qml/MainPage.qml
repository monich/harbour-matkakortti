import QtQuick 2.0
import Sailfish.Silica 1.0
import org.nemomobile.configuration 1.0
import harbour.matkakortti 1.0

import "components"

Page {
    id: page

    backNavigation: false
    showNavigationIndicator: false

    readonly property string settingsPath: "/apps/harbour-matkakortti/"
    readonly property url hmmImageUrl: "image://harbour/" + Qt.resolvedUrl("images/hmm.svg") + "?" + Theme.highlightColor
    readonly property bool targetPresent: NfcAdapter.targetPresent
    readonly property bool unrecorgnizedCard: targetPresent && travelCard.cardState === TravelCard.CardNone && !readTimer.running
    readonly property bool readingCard: travelCard.cardState === TravelCard.CardReading || readTimer.running
    readonly property bool nysseSupported: NfcSystem.version >= NfcSystem.MinimumVersionForNysseSupport
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

        key: settingsPath + "lastCardType"
    }

    TravelCard {
        id: travelCard

        path: NfcAdapter.tagPath
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

    SilicaFlickable {
        anchors.fill: parent

        Item {
            anchors.fill: parent
            opacity: (NfcSystem.valid && (!NfcSystem.present || !NfcAdapter.present)) ? 1 : 0
            visible: opacity > 0

            Behavior on opacity { FadeAnimation {} }

            Image {
                y: (parent.height/2 - height)/2
                anchors.horizontalCenter: parent.horizontalCenter
                height: page.width/2
                sourceSize.height: height
                source: parent.visible ? hmmImageUrl : ""
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
            opacity: (NfcSystem.valid && NfcSystem.present && NfcAdapter.present) ? 1 : 0
            visible: opacity > 0

            Behavior on opacity { FadeAnimation {} }

            Item {
                width: parent.width
                height: parent.height/2
                opacity: (!readingCard && !targetPresent) ? 1 : 0
                visible: opacity > 0

                property real cardImageHeight: Math.round(2*width/5)

                Behavior on opacity { FadeAnimation {} }

                CardImage {
                    anchors.centerIn: parent
                    sourceSize.height: parent.cardImageHeight
                    source: Qt.resolvedUrl("nysse/images/nysse-card.svg")
                    rotation: 105
                    visible: nysseSupported
                    z: (lastCardType.value === "Nysse") ? 1 : 0
                }

                CardImage {
                    anchors.centerIn: parent
                    sourceSize.height: parent.cardImageHeight
                    source: Qt.resolvedUrl("hsl/images/hsl-card.svg")
                    rotation: nysseSupported ? 60 : 90
                    z: (lastCardType.value === "HSL") ? 1 : 0
                }
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

            Image {
                y: (parent.height/2 - height)/2
                anchors.horizontalCenter: parent.horizontalCenter
                height: page.width/2
                sourceSize.height: height
                source: visible ? hmmImageUrl : ""
                smooth: true
                opacity: (!readingCard && unrecorgnizedCard) ? 1 : 0
                visible: opacity > 0
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
                        //% "This is not an HSL travel card"
                        qsTrId("matkakortti-info-card_not_supported") : "") :
                    (NfcSystem.enabled ?
                        //: Info label
                        //% "Ready"
                        qsTrId("matkakortti-info-ready") :
                        //: Info label
                        //% "NFC is disabled"
                        qsTrId("matkakortti-info-disabled"))
            }

            Text {
                x: statusLabel.x
                y: Math.floor((parent.height * 3 /2 - height)/2)
                width: statusLabel.width
                horizontalAlignment: Text.AlignHCenter
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
                Behavior on opacity { FadeAnimation {} }
            }
        }
    }
}

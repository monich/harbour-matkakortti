import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

Page {
    id: page

    backNavigation: false
    showNavigationIndicator: false

    readonly property url hmmImageUrl: "image://harbour/" + Qt.resolvedUrl("images/hmm.svg") + "?" + Theme.highlightColor
    readonly property bool targetPresent: NfcAdapter.targetPresent
    readonly property bool unrecorgnizedCard: targetPresent && hslCard.valid && (!hslCard.present || hslCard.failed)
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

    HslCard {
        id: hslCard

        path: NfcAdapter.tagPath
        onFailedChanged: {
            if (failed && cardInfoPage) {
                cardInfoPage.backNavigation = true
                pageStack.pop(page, PageStackAction.Animated)
            }
        }
        onHslTravelCard: {
            console.log("appInfo:",appInfo)
            console.log("controlInfo:",controlInfo)
            console.log("periodPass:",periodPass)
            console.log("storedValue:",storedValue)
            console.log("eTicket:",eTicket)
            console.log("history:",history)
            if (cardInfoPage) {
                cardInfoPage.appInfo = appInfo
                cardInfoPage.controlInfo = controlInfo
                cardInfoPage.periodPass = periodPass
                cardInfoPage.storedValue = storedValue
                cardInfoPage.eTicket = eTicket
                cardInfoPage.history = history
            } else {
                cardInfoPage = pageStack.push(Qt.resolvedUrl("TravelCardPage.qml"), {
                    appInfo: appInfo,
                    controlInfo: controlInfo,
                    periodPass: periodPass,
                    storedValue: storedValue,
                    eTicket: eTicket,
                    history: history
                })
                if (cardInfoPage) {
                    cardInfoPage.statusChanged.connect(function() {
                        if (cardInfoPage.status === PageStatus.Inactive) {
                            cardInfoPage.destroy()
                            cardInfoPage = null
                        }
                    })
                }
            }
        }
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

            readonly property bool unsupported: hslCard.valid && (!hslCard.present || hslCard.failed)

            Behavior on opacity { FadeAnimation {} }

            Image {
                y: (parent.height/2 - height)/2
                anchors.horizontalCenter: parent.horizontalCenter
                height: page.width/2
                sourceSize.height: height
                source: Qt.resolvedUrl("images/app-icon.svg")
                smooth: true
                opacity: targetPresent ? 0 : 1
                visible: opacity > 0
                Behavior on opacity { FadeAnimation {} }
            }

            BusyIndicator {
                y: (parent.height/2 - height)/2
                anchors.horizontalCenter: parent.horizontalCenter
                size: BusyIndicatorSize.Large
                running: true
                opacity: (targetPresent && !parent.unsupported) ? 1 : 0
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
                opacity: (targetPresent && parent.unsupported) ? 1 : 0
                visible: opacity > 0
            }

            InfoLabel {
                id: statusLabel

                y: parent.height/2

                text: targetPresent ? (
                    hslCard.valid ? ((!hslCard.present || hslCard.failed) ?
                    //: Info label
                    //% "This is not an HSL travel card"
                    qsTrId("matkakortti-info-card_not_supported") :
                    //: Info label
                    //% "Reading the card"
                    qsTrId("matkakortti-info-reading")) : "") :
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
                opacity: (NfcSystem.enabled && !targetPresent) ? HarbourTheme.opacityHigh : 0
                visible: opacity > 0
                Behavior on opacity { FadeAnimation {} }
            }
        }
    }
}

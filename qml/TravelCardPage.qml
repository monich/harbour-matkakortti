import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

Page {
    id: page

    property alias appInfo: hslCardAppInfo.data
    property string controlInfo
    property alias periodPass: details.periodPass
    property alias storedValue: details.storedValue
    property alias eTicket: details.eTicket
    property string history

    property alias moneyAmount: details.moneyAmount
    property alias ticketSecondsRemaining: details.ticketSecondsRemaining
    property alias periodPassDaysRemaining: details.periodPassDaysRemaining
    property alias periodPassEndDate: details.periodPassEndDate

    showNavigationIndicator: false

    onStatusChanged: {
        if (status === PageStatus.Active) {
            backNavigation = !NfcAdapter.targetPresent
            showNavigationIndicator = true
        }
    }

    HslCardAppInfo {
        id: hslCardAppInfo
    }

    PageHeader {
        id: header

        //: Page title
        //% "Travel card"
        title: qsTrId("matkakortti-card-header")
        description: hslCardAppInfo.cardNumber

        Image {
            anchors {
                left: header.extraContent.left
                verticalCenter: header.verticalCenter
            }
            height: header.height/2
            sourceSize.height: height
            source: "images/hsl-card.svg"
            smooth: true
        }
    }

    TravelCardDetails {
        id: details

        anchors {
            top: header.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
    }
}

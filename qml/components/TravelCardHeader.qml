import QtQuick 2.0
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0

PageHeader {
    id: header

    property alias cardImageUrl: headerImage.source
    property string cardType

    signal multiClick()

    title: cardType ?
        //: Page title
        //% "%1 card"
        qsTrId("matkakortti-card_type-header").arg(cardType) :
        //: Page title
        //% "Travel card"
        qsTrId("matkakortti-card-header")

    Rectangle {
        id: headerImageBackground

        color: Theme.rgba(Theme.highlightColor, 0.8 /* opacityOverlay */)
        height: headerImage.height + Theme.paddingSmall
        width: headerImage.width + Theme.paddingSmall
        radius: Theme.paddingSmall
        anchors {
            left: header.extraContent.left
            verticalCenter: header.verticalCenter
        }
    }

    Image {
        id: headerImageMask

        anchors.fill: headerImage
        sourceSize: Qt.size(headerImage.width, headerImage.height)
        source: Qt.resolvedUrl("../images/card-mask.svg")
        smooth: true
        visible: false
    }

    Image {
        id: headerImage

        anchors.centerIn: headerImageBackground
        height: header.height/2
        sourceSize.height: height
        source: cardImageUrl
        smooth: true
        visible: false
    }

    OpacityMask {
        anchors.fill: headerImage
        source: headerImage
        maskSource: headerImageMask
    }

    MouseArea {
        id: multiClickArea

        property int clickCount
        readonly property int multiClickCount: 5

        anchors {
            right: parent.right
            left: parent.horizontalCenter
            top: parent.top
            bottom: parent.bottom
        }

        onClicked: handleClick()
        onDoubleClicked: handleClick()

        function handleClick() {
            clickCount ++
            if (clickCount >= multiClickCount) {
                multiClickResetTimer.stop()
                clickCount = 0
                header.multiClick()
            } else {
                multiClickResetTimer.restart()
            }
        }

        Timer {
            id: multiClickResetTimer

            interval: 500
            onTriggered: multiClickArea.clickCount = 0
        }
    }
}

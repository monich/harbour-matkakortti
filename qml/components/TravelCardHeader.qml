import QtQuick 2.0
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

PageHeader {
    id: header

    property alias cardImageUrl: headerImage.source

    //: Page title
    //% "Travel card"
    title: qsTrId("matkakortti-card-header")

    Rectangle {
        id: headerImageBackground

        color: Theme.rgba(Theme.highlightColor, HarbourTheme.opacityOverlay)
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
}

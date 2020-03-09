import QtQuick 2.0
import QtGraphicalEffects 1.0

Item {
    id: item

    height: image.height
    width: image.width

    property alias source: image.source
    property alias sourceSize: image.sourceSize

    Image {
        id: mask

        anchors.fill: image
        sourceSize: Qt.size(image.width, image.height)
        source: Qt.resolvedUrl("../images/card-mask.svg")
        smooth: true
        visible: false
    }

    Image {
        id: image

        anchors.centerIn: parent
        sourceSize.height: height
        smooth: true
        visible: false
    }

    OpacityMask {
        anchors.fill: image
        source: image
        maskSource: mask
    }
}

import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

ShaderEffectSource {
    property alias source: image.source
    property alias sourceSize: image.sourceSize

    height: item.height
    width: item.width

    sourceItem: Item {
        id: item

        height: image.height + 2 * Theme.paddingLarge
        width: image.width + 2 * Theme.paddingLarge

        Image {
            id: shadow

            anchors.centerIn: parent
            sourceSize: Qt.size(image.width, image.height)
            source: Qt.resolvedUrl("../images/card-mask.svg")
            smooth: true
            visible: false
        }

        FastBlur {
            source: shadow
            anchors.fill: shadow
            radius: 32
            transparentBorder: true
        }

        CardImage {
            id: image

            anchors.centerIn: parent
        }
    }
}

import QtQuick 2.0
import Sailfish.Silica 1.0

Item {
    id: switcher

    height: highlighter.height
    implicitWidth: buttonWidth + Theme.paddingLarge

    property Item list
    property alias leftLabel: leftButton.text
    property alias rightLabel: rightButton.text
    readonly property bool animating: scrollAnimation.running
    readonly property real buttonWidth: Math.max(leftButton.implicitWidth, rightButton.implicitWidth) + 2 * Theme.paddingSmall

    Rectangle {
        id: highlighter

        readonly property real padding: Theme.paddingMedium
        x: leftButton.x - padding + (rightButton.x -  leftButton.x) * (list.contentX - list.originX) / list.width
        anchors.verticalCenter: parent.verticalCenter
        radius: Theme.paddingMedium
        width: buttonWidth + 2 * padding
        height: Math.max(leftButton.height, rightButton.height)
        color: Theme.primaryColor
        opacity: 0.2 // opacityFaint
    }

    PressableLabel {
        id: leftButton

        anchors {
            verticalCenter: parent.verticalCenter
            right: rightButton.left
            rightMargin: Theme.paddingLarge
        }
        width: buttonWidth
        highlighted: list.currentIndex === 0
        onClicked: { // Animate positionViewAtBeginning()
            if (!scrollAnimation.running && list.contentX > list.originX) {
                scrollAnimation.from = list.contentX
                scrollAnimation.to = list.originX
                scrollAnimation.start()
            }
        }
    }

    PressableLabel {
        id: rightButton

        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
        }
        width: buttonWidth
        highlighted: list.currentIndex === 1
        onClicked: { // Animate positionViewAtEnd()
            if (!scrollAnimation.running && list.contentX < list.maxContentX) {
                scrollAnimation.from = list.contentX
                scrollAnimation.to = list.maxContentX
                scrollAnimation.start()
            }
        }
    }

    NumberAnimation {
        id: scrollAnimation

        target: list
        property: "contentX"
        duration: 200
        easing.type: Easing.InOutQuad
    }
}

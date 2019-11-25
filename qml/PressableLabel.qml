import QtQuick 2.0
import Sailfish.Silica 1.0

MouseArea {
    property alias text: buttonText.text
    property bool highlighted
    readonly property int minimumPressHighlightTime: 64
    readonly property bool showHighlight: highlighted || (pressed && containsMouse) || pressTimer.running

    implicitWidth: buttonText.implicitWidth
    implicitHeight: buttonText.implicitHeight
    width: Math.max(implicitWidth, Theme.buttonWidthSmall)
    height: Math.max(implicitHeight, Theme.itemSizeExtraSmall)

    onPressedChanged: if (pressed) pressTimer.start()
    onCanceled: pressTimer.stop()

    Label {
        id: buttonText

        width: Math.min(parent.width, implicitWidth)
        anchors.centerIn: parent
        truncationMode: TruncationMode.Fade
        color: showHighlight ? Theme.highlightColor : Theme.primaryColor
    }

    Timer {
        id: pressTimer

        interval: minimumPressHighlightTime
    }
}

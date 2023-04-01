import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    property alias title: titleText.text
    property alias value: valueText.text
    property alias suffix: suffixText.text
    property alias boldValue: valueText.font.bold
    property int fontSize: Theme.fontSizeMedium
    readonly property real preferredWidth: titleText.preferredWidth + valueText.preferredWidth + suffixText.preferredWidth

    Flow {
        width: parent.width

        Label {
            id: titleText

            readonly property real preferredWidth: implicitWidth + Theme.paddingMedium
            width: Math.min(preferredWidth, parent.width)
            font.pixelSize: fontSize
            color: Theme.secondaryHighlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
        }

        Label {
            id: valueText

            readonly property real preferredWidth: implicitWidth + (suffix ? Theme.paddingMedium : 0)
            width: Math.min(preferredWidth, parent.width - x)
            font.pixelSize: fontSize
            color: Theme.highlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
        }

        Label {
            id: suffixText

            readonly property real preferredWidth: implicitWidth
            width: Math.min(implicitWidth, parent.width - x)
            font.pixelSize: fontSize
            color: Theme.secondaryHighlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
        }
    }
}

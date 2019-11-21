import QtQuick 2.0
import Sailfish.Silica 1.0

Column {
    property alias title: titleText.text
    property alias value: valueText.text
    property alias suffix: suffixText.text

    Flow {
        width: parent.width

        Label {
            id: titleText

            width: Math.min(implicitWidth + Theme.paddingMedium, parent.width)
            color: Theme.secondaryHighlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
        }

        Label {
            id: valueText

            width: Math.min(implicitWidth + (suffix ? Theme.paddingMedium : 0), parent.width)
            color: Theme.highlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
        }

        Label {
            id: suffixText

            width: Math.min(implicitWidth, parent.width)
            color: Theme.secondaryHighlightColor
            horizontalAlignment: Text.AlignLeft
            truncationMode: TruncationMode.Fade
        }
    }
}

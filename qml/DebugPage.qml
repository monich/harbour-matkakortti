import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    property var debug

    PageHeader {
        id: header

        title: "Debug"
    }

    SilicaFlickable {
        anchors {
            top: header.bottom
            bottom: parent.bottom
        }
        width: parent.width
        contentHeight: textArea.height
        clip: true

        TextArea {
            id: textArea

            font {
                pixelSize: Theme.fontSizeTiny
                family: "Monospace"
            }
            width: parent.width
            readOnly: false
            wrapMode: TextEdit.WrapAnywhere
            backgroundStyle: TextEditor.FilledBackground
            softwareInputPanelEnabled: false
            focus: false
            text: (debug && debug.log) ? debug.log : ""
            visible: debug && debug.log
        }

        VerticalScrollDecorator { }
    }
}

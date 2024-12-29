import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

Page {
    id: thisPage

    property var debug

    readonly property bool _haveDebugLog: debug && debug.log
    readonly property string _debugLog: _haveDebugLog ? debug.log : ""
    readonly property string _transferEngineVersion: HarbourSystemInfo.packageVersion("declarative-transferengine-qt5")
    readonly property bool _canShare: _haveDebugLog && HarbourSystemInfo.compareVersions(_transferEngineVersion, "0.4.0") >= 0

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: height

        PullDownMenu {
            visible: _haveDebugLog

            MenuItem {
                //: Generic menu item, shares the content
                //% "Share"
                text: qsTrId("matkakortti-menu-share")
                visible: _canShare
                property var action
                onClicked: {
                    if (!action) {
                        action = Qt.createQmlObject("import Sailfish.Share 1.0;ShareAction{mimeType: 'text/plain'}",
                            thisPage, "SailfishShare")
                    }
                    action.resources = [{ "data": debug.log, "name": "matkakortti.log" }]
                    action.trigger()
                }
            }
            //: Generic menu item, copies text to clipboard
            //% "Copy to clipboard"
            MenuItem {
                text: qsTrId("matkakortti-menu-copy_to_clipboard")
                visible: _haveDebugLog
                onClicked: Clipboard.text = _debugLog
            }
        }

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
                text: _debugLog
                visible: _haveDebugLog
            }

            VerticalScrollDecorator { }
        }
    }
}

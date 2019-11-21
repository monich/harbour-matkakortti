import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

Item {
    id: item

    width: background.width
    height: background.height

    property int valid

    Rectangle {
        id: background

        color: "white"
        opacity: HarbourTheme.opacityFaint
        radius: Theme.paddingMedium
        width: validityLabel.paintedWidth + 2 * Theme.paddingMedium
        height: validityLabel.height + 2 * Theme.paddingMedium
    }

    Label {
        id: validityLabel

        width: Theme.itemSizeHuge
        anchors {
            top: background.top
            right: background.right
            margins: Theme.paddingMedium
        }
        horizontalAlignment: Text.AlignRight
        font.bold: true
        wrapMode: Text.Wrap
        color: valid > 0 ? "green" : "red"
        //: Validity label
        //% "Valid"
        text: (valid > 0) ? qsTrId("matkakortti-card-validity-valid") :
            //: Validity label
            //% "Not yet valid"
            (valid === HslData.PeriodNotYetStarted) ? qsTrId("matkakortti-card-validity-not_yet_valid") :
            //: Validity label
            //% "No longer valid"
            (valid === HslData.PeriodEnded) ? qsTrId("matkakortti-card-validity-no_longer_valid") : ""
    }
}

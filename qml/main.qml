import QtQuick 2.0
import Sailfish.Silica 1.0
import harbour.matkakortti 1.0

ApplicationWindow {
    id: appWindow

    allowedOrientations: Orientation.Portrait

    property bool cardInfoShown

    //: Application title
    //% "Matkakortti"
    readonly property string title: qsTrId("matkakortti-app_name")

    initialPage: MainPage { id: mainPage }
    cover: CoverPage {
        cardInfoPage: mainPage.cardInfoPage
        unrecorgnizedCard: mainPage.unrecorgnizedCard
        onPopCardInfo: pageStack.pop(mainPage, PageStackAction.Immediate)
    }

    Connections {
        target: HarbourSystemTime
        onPreNotify: Date.timeZoneUpdated()
    }
}

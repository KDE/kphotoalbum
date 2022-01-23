import QtQuick 2.0
import QtQuick.Controls 1.4 as QQC1 // We need Menu from QQC 1, not 2!
import QtQuick.Extras 1.4

PieMenu {
    id: root
    property int imageId : -1
    property bool imageViewer : false

    triggerMode: TriggerMode.TriggerOnRelease

    QQC1.MenuItem {
        text: "Run Slide Show"
        iconSource: "qrc:/Images/view-presentation.png"
        onTriggered: {
            _slideShow.running = true
            if (!imageViewer)
                _remoteInterface.showImage(imageId)
        }
    }

//        QQC1.MenuItem {
//            text: "Show Time Line"
//            onTriggered: console.log("Show timeline")
//        }

    QQC1.MenuItem {
        visible: !imageViewer
        text: "Refine Search"
        iconSource: "qrc:/Images/system-search.png"
        onTriggered: _remoteInterface.showOverviewPage()
    }

    QQC1.MenuItem {
        text: "Go Home"
        iconSource: root.visible ? "image://images/home" : ""
        onTriggered: _remoteInterface.goHome()
    }
}

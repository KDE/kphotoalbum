import QtQuick 2.2
import SlideViewer 1.0

Item {
    focus: true

    RemoteImage {
        id: remoteImage
        anchors.fill: parent
        visible: connected
    }

    Text {
        visible: !remoteImage.connected
        text: "Not Connceted"
        anchors.centerIn: parent
        font.pixelSize: 50
    }

    SwipeArea {
        anchors.fill: parent
        onSwipeLeft: _remoteInterface.nextSlide()
        onSwipeRight: _remoteInterface.previousSlide()
    }

//    MouseArea {
//        anchors {
//            left: parent.left
//            right: parent.horizontalCenter
//            top: parent.top
//            bottom: parent.bottom
//        }
//        onClicked: _remoteInterface.previousSlide()
//    }

//    MouseArea {
//        anchors {
//            left: parent.horizontalCenter
//            right: parent.right
//            top: parent.top
//            bottom: parent.bottom
//        }
//        onClicked: _remoteInterface.nextSlide()
//    }

    Keys.onPressed: {
        if ( event.key == Qt.Key_Q && (event.modifiers & Qt.ControlModifier ) )
            Qt.quit()
    }
}

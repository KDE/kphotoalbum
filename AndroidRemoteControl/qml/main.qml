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

    Keys.onPressed: {
        if ( event.key == Qt.Key_Q && (event.modifiers & Qt.ControlModifier ) )
            Qt.quit()
    }
}

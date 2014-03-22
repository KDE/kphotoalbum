import QtQuick 2.2
import KPhotoAlbum 1.0

Item {
    id: root
    focus: true

    RemoteImage {
        id: remoteImage
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

    PinchArea {
        pinch.target: remoteImage
        anchors.fill: parent
        pinch.minimumScale: 0.1
        pinch.maximumScale: 10
        pinch.dragAxis: Pinch.XandYAxis
    }
}

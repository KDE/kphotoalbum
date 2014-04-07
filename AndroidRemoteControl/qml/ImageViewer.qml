import QtQuick 2.0
import KPhotoAlbum 1.0

ListView {
    id: root
    model: _remoteInterface.thumbnails
    snapMode: ListView.SnapToItem
    orientation: ListView.Horizontal
    flickDeceleration: 2000

    delegate: RemoteImage {
        width: root.width
        height: root.height
        fileName: modelData
        isThumbnail: false
        PinchArea {
            pinch.target: parent
            anchors.fill: parent
            pinch.minimumScale: 0.1
            pinch.maximumScale: 10
            pinch.dragAxis: Pinch.XandYAxis
        }
    }
}

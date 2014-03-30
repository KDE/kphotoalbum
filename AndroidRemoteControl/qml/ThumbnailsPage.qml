import QtQuick 2.0
import KPhotoAlbum 1.0

PinchArea {
    pinch.minimumScale: 0.1
    pinch.maximumScale: 10
    onPinchUpdated: grid.scale = pinch.scale
    onPinchFinished: _settings.thumbnailScale = scale
    GridView {
        id: grid
        anchors.fill: parent
        model: _remoteInterface.thumbnails
        cellWidth: 210
        cellHeight: 210
        delegate: RemoteImage { fileName: modelData }
    }
}

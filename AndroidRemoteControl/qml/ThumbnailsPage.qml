import QtQuick 2.0
import KPhotoAlbum 1.0

PinchArea {
    pinch.minimumScale: 0.1
    pinch.maximumScale: 10
    onPinchUpdated: grid.scale = pinch.scale
    onPinchFinished: {
        _settings.thumbnailSize = pinch.scale * _settings.thumbnailSize
        grid.scale = 1
    }
    GridView {
        id: grid
        anchors.fill: parent
        transformOrigin: Qt.TopLeftCorner
        model: _remoteInterface.thumbnails
        cellWidth: _settings.thumbnailSize + 10
        cellHeight: _settings.thumbnailSize + 10
        delegate: RemoteImage { fileName: modelData }
    }
}

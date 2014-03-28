import QtQuick 2.0
import KPhotoAlbum 1.0

GridView {
    model: _remoteInterface.thumbnails
    cellWidth: 210
    cellHeight: 210
    delegate: RemoteImage { fileName: modelData }
}

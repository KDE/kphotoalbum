import QtQuick 2.0
import KPhotoAlbum 1.0

PinchArea {
    id: root
    property alias model : grid.model
    property int type // FIXME should be an enum!
    property bool showLabels : false
    signal clicked(int value)

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
        cellWidth: _settings.thumbnailSize + 10
        cellHeight: _settings.thumbnailSize + 10 + (root.showLabels ? 30 : 0)
        delegate:
            Column {
            RemoteImage {
                id: remoteImage
                imageId: model.imageId
                type: root.type
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.clicked(parent.imageId)
                }
            }
            Text {
                visible: root.showLabels
                anchors { left: parent.left; right: parent.right }
                text: remoteImage.label
                elide: Text.ElideLeft
            }
        }
    }
    ScrollBar {
        flickable: grid
    }
}

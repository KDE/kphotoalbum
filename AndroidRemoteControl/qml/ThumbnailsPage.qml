import QtQuick 2.0
import KPhotoAlbum 1.0

PinchArea {
    id: root
    property alias model : grid.model
    property int type
    property bool showLabels : false
    readonly property int itemsPerPage : Math.floor(root.width/grid.cellWidth) * Math.floor(root.height/grid.cellHeight)
    signal clicked(int imageId, string label)

    pinch.minimumScale: 0.1
    pinch.maximumScale: 10
    onPinchUpdated: grid.scale = pinch.scale
    onPinchFinished: {
        if ( type == Enums.CategoryItems )
            _settings.categoryItemSize = pinch.scale * _settings.categoryItemSize
        else
            _settings.thumbnailSize = pinch.scale * _settings.thumbnailSize
        grid.scale = 1
    }

    GridView {
        id: grid
        anchors.fill: parent
        transformOrigin: Qt.TopLeftCorner

        cellWidth: imageWidth() + padding()
        cellHeight: imageWidth() + padding() + (root.showLabels ? 30 : 0)

        delegate:
            Column {

            RemoteImage {
                x: padding()/2
                id: remoteImage
                imageId: model.imageId
                type: root.type

                width: imageWidth()
                height: width
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.clicked(parent.imageId,parent.label)
                }
            }
            Text {
                visible: root.showLabels
                anchors { left: parent.left; right: parent.right; margins: padding()/2 }
                text: remoteImage.label
                elide: Text.ElideRight
                verticalAlignment: Text.AlignTop
            }
        }
    }
    ScrollBar {
        flickable: grid
    }

    function imageWidth() {
        return type == Enums.CategoryItems ? _settings.categoryItemSize : _settings.thumbnailSize;
    }
    function padding() {
        return imageWidth()/8
    }
}

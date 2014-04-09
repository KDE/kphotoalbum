import QtQuick 2.0
import KPhotoAlbum 1.0

ListView {
    id: root
    model: _remoteInterface.thumbnails
    snapMode: ListView.SnapToItem
    orientation: ListView.Horizontal
    flickDeceleration: 20000
    highlightMoveDuration: 200
    //highlightRangeMode: ListView.StrictlyEnforceRange
    interactive: currentItem && currentItem.scale <= 1 // Only swipe when zoomed out

    delegate: RemoteImage {
        width: root.width
        height: root.height
        fileName: modelData
        isThumbnail: false
        PinchArea {
            pinch.target: parent
            anchors.fill: parent
            pinch.minimumScale: 1
            pinch.maximumScale: 10
            pinch.dragAxis: Pinch.XandYAxis
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if ( mouse.x < root.width/2 )
                    root.decrementCurrentIndex()
                else
                    root.incrementCurrentIndex()
            }
        }
    }

    Connections {
        target: _remoteInterface
        onJumpToImage: {
            var tmp = root.highlightMoveDuration;
            root.highlightMoveDuration = 0
            root.currentIndex = index
            root.highlightMoveDuration = tmp
        }
    }
}

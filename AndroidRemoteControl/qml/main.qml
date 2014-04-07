import QtQuick 2.2
import KPhotoAlbum 1.0

Item {
    OverviewPage {
        anchors.fill: parent
        visible: _remoteInterface.currentPage == "Overview"
    }

    CategoryItemsPage {
        anchors.fill: parent
        visible: _remoteInterface.currentPage == "CategoryItems"
    }

    ThumbnailsPage {
        anchors.fill: parent
        visible: _remoteInterface.currentPage == "Thumbnails"
    }

    ImageViewer {
        anchors.fill: parent
        visible: _remoteInterface.currentPage == "ImageViewer"
    }

    Text {
        visible: _remoteInterface.currentPage == "Unconnected"
        text: "Not Connceted"
        anchors.centerIn: parent
        font.pixelSize: 50
    }

    focus: true
    Keys.onReleased: {
        if ( event.key == Qt.Key_Q && (event.modifiers & Qt.ControlModifier ) )
            Qt.quit()
        if ( event.key == Qt.Key_Back) {
            _remoteInterface.goBack()
            event.accepted = true
        }
    }
}

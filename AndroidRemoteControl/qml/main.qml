import QtQuick 2.2
import KPhotoAlbum 1.0

Item {
    OverviewPage {
        anchors.fill: parent
        visible: _remoteInterface.currentPage == Page.Overview
    }

    // FIXME: REMOVE class
//    CategoryItemsPage {
//        anchors.fill: parent
//        visible: _remoteInterface.currentPage == "CategoryItems"
//    }

    ThumbnailsPage {
        visible: _remoteInterface.currentPage == Page.CategoryItems
        anchors.fill: parent
        model: _remoteInterface.categoryItems
        type: 0 // FIXME: ViewType::CategoryItems
        showLabels: true
        onClicked: _remoteInterface.selectCategoryValue(label)
    }

    ThumbnailsPage {
        visible: _remoteInterface.currentPage == Page.Thumbnails
        anchors.fill: parent
        model: _remoteInterface.thumbnailModel
        type: 1 // FIXME ViewType::Thumbnails
        showLabels: false
        onClicked: _remoteInterface.showImage(imageId)
    }

    ImageViewer {
        anchors.fill: parent
        visible: _remoteInterface.currentPage == Page.ImageViewer
    }

    Text {
        visible: _remoteInterface.currentPage == Page.Unconnected
        text: "Not Connceted"
        anchors.centerIn: parent
        font.pixelSize: 50
    }

    focus: true
    Keys.onReleased: {
        if ( event.key == Qt.Key_Q && (event.modifiers & Qt.ControlModifier ) )
            Qt.quit()
        if (event.key == Qt.Key_Back || event.key == Qt.Key_Left) {
            _remoteInterface.goBack()
            event.accepted = true
        }
        if (event.key == Qt.Key_Right)
            _remoteInterface.goForward()
    }
}

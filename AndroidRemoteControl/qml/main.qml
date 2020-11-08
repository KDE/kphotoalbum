/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import KPhotoAlbum 1.0

Rectangle {
    color: _settings.backgroundColor

    OverviewPage {
        anchors.fill: parent
        visible: _remoteInterface.currentPage === Enums.OverviewPage
    }

    ThumbnailsPage {
        name: "categoryPage"
        visible: _remoteInterface.currentPage === Enums.CategoryItemsPage
        anchors.fill: parent
        model: visible ? _remoteInterface.categoryItems : undefined
        type: Enums.CategoryItems
        showLabels: true
        onClicked: _remoteInterface.selectCategoryValue(label)
    }

    CategoryListView {
        visible: _remoteInterface.currentPage === Enums.CategoryListPage
        anchors.fill: parent
        model: visible ? _remoteInterface.listCategoryValues : undefined
        onClicked: _remoteInterface.selectCategoryValue(label)
    }

    ThumbnailsPage {
        id: thumbnailsPage
        name: "thumbnailsPage"
        visible: _remoteInterface.currentPage === Enums.ThumbnailsPage
        anchors.fill: parent
        model: _remoteInterface.thumbnailModel
        type: Enums.Thumbnails
        showLabels: false
        onClicked: _remoteInterface.showImage(imageId)
    }

    ThumbnailsPage {
        id: discoveryPage
        visible: _remoteInterface.currentPage === Enums.DiscoverPage
        anchors.fill: parent
        model: _remoteInterface.discoveryModel
        type: Enums.Thumbnails
        showLabels: false
        onClicked: {
            if (imageId === -1000 /* DISCOVERYID*/)
                model.resetImages()
            else
                _remoteInterface.showImage(imageId)
        }

        Binding {
            target: _remoteInterface.discoveryModel
            property: "count"
            value: discoveryPage.itemsPerPage-1
            when: discoveryPage.visible
        }
     }

    ImageViewer {
        id: imageViewer
        anchors.fill: parent
        visible: _remoteInterface.currentPage === Enums.ImageViewerPage
    }

    Text {
        visible: _remoteInterface.currentPage === Enums.UnconnectedPage
        width: parent.width*6/7
        wrapMode: Text.WordWrap
        color: _settings.textColor

        text: "Not Connected\nMake sure you have a running KPhotoAlbum on the same network.\nMy address: " + _remoteInterface.networkAddress
        anchors.centerIn: parent
        font.pixelSize: 50
    }

    Binding {
        target: _screenInfo
        property: "viewWidth"
        value: width
    }

    Binding {
        target: _screenInfo
        property: "viewHeight"
        value: height
    }

    // Dummy elemenent so I can know the default text element hight
    Text {
        id: dummy
        visible: false
        text: "Hi"
        Binding {
            target: _screenInfo
            property: "textHeight"
            value: dummy.height
        }
    }


    focus: true
    // allow subpages to have Keyboard handlers:
    Keys.forwardTo: [ thumbnailsPage, discoveryPage, imageViewer ]
    Keys.onPressed: {
        if ( event.key === Qt.Key_Q && (event.modifiers & Qt.ControlModifier ) )
            Qt.quit()
        if (event.key === Qt.Key_Back || event.key === Qt.Key_Left) {
            _remoteInterface.goBack()
            event.accepted = true
        }
        if (event.key === Qt.Key_Right) {
            _remoteInterface.goForward()
            event.accepted = true
        }
    }
}

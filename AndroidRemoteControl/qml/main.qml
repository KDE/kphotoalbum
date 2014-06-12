/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.2
import KPhotoAlbum 1.0

Rectangle {
    color: _settings.backgroundColor

    OverviewPage {
        anchors.fill: parent
        visible: _remoteInterface.currentPage == Enums.OverviewPage
    }

    ThumbnailsPage {
        name: "categoryPage"
        visible: _remoteInterface.currentPage == Enums.CategoryItemsPage
        anchors.fill: parent
        model: visible ? _remoteInterface.categoryItems : undefined
        type: Enums.CategoryItems
        showLabels: true
        onClicked: _remoteInterface.selectCategoryValue(label)
    }

    CategoryListView {
        visible: _remoteInterface.currentPage == Enums.CategoryListPage
        anchors.fill: parent
        model: visible ? _remoteInterface.listCategoryValues : undefined
        onClicked: _remoteInterface.selectCategoryValue(label)
    }

    ThumbnailsPage {
        name: "thumbnailsPage"
        visible: _remoteInterface.currentPage == Enums.ThumbnailsPage
        anchors.fill: parent
        model: _remoteInterface.thumbnailModel
        type: Enums.Thumbnails
        showLabels: false
        onClicked: _remoteInterface.showImage(imageId)
    }

    ThumbnailsPage {
        id: discoveryPage
        visible: _remoteInterface.currentPage == Enums.DiscoverPage
        anchors.fill: parent
        model: _remoteInterface.discoveryModel
        type: Enums.Thumbnails
        showLabels: false
        onClicked: {
            if (imageId == -1000 /* DISCOVERYID*/)
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
        anchors.fill: parent
        visible: _remoteInterface.currentPage == Enums.ImageViewerPage
    }

    Text {
        visible: _remoteInterface.currentPage == Enums.UnconnectedPage
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

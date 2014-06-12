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

import QtQuick 2.0
import KPhotoAlbum 1.0
import QtQuick.Controls 1.1

PinchArea {
    id: root
    property alias model : grid.model
    property int type
    property bool showLabels : false
    property alias name : observer.objectName
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

        delegate: Item {
            Column {
                x: (padding() + grid.cellWidth - width)/2
                y: grid.cellHeight - height

                RemoteImage {
                    id: remoteImage
                    imageId: model.imageId
                    type: root.type

                    width: imageWidth()
                    height: width
                    MouseArea {
                        anchors.fill: parent
                        onClicked: root.clicked(parent.imageId,parent.label)
                        onPressAndHold: menu.popup()
                    }
                }
                Text {
                    visible: root.showLabels
                    color: _settings.textColor
                    anchors { left: parent.left; right: parent.right; margins: padding()/2 }
                    text: remoteImage.label
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignTop
                }
            }
        }
    }
    ScrollBar {
        flickable: grid
    }

    Menu {
        id: menu
        title: "Context Menu"
        MenuItem {
            text: "Narrow"
            onTriggered: _remoteInterface.showOverviewPage()
        }
        MenuItem {
            text: "Go Home"
            onTriggered: _remoteInterface.goHome()
        }
    }

    PositionObserver {
        id: observer
        view: grid
    }

    function imageWidth() {
        return type == Enums.CategoryItems ? _settings.categoryItemSize : _settings.thumbnailSize;
    }
    function padding() {
        return 20 // imageWidth()/8 - rhk would like us to try not to scale the space.
    }
}

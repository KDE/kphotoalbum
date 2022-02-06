/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import KPhotoAlbum 1.0
import QtQuick.Controls 1.4 as QQC1 // We need Menu from QQC 1, not 2!
import QtQuick.Extras 1.4
import QtQuick.Controls 2.15 as QQC2

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
            property alias imageId: remoteImage.imageId
            Column {
                x: (root.padding() + grid.cellWidth - width)/2
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
                    }
                }
                Text {
                    visible: root.showLabels
                    color: _settings.textColor
                    anchors { left: parent.left; right: parent.right; margins: root.padding()/2 }
                    text: remoteImage.label
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignTop
                }
            }
        }
        PressAndHoldArea {
            anchors.fill: parent
            onPressAndHold: {
                var child = grid.itemAt(grid.contentX + mouseX, grid.contentY + mouseY)
                contextMenu.imageId = child ? child.imageId : -1
                contextMenu.popup(mouseX, mouseY)
            }
        }
        QQC2.ScrollBar.vertical: QQC2.ScrollBar {
            width: _screenInfo.dotsPerMM * 5
        }
    }

    PositionObserver {
        id: observer
        view: grid
    }

    ContextMenu {
        id: contextMenu
    }

    function imageWidth() {
        return type == Enums.CategoryItems ? _settings.categoryItemSize : _settings.thumbnailSize;
    }
    function padding() {
        return 20 // imageWidth()/8 - rhk would like us to try not to scale the space.
    }
}

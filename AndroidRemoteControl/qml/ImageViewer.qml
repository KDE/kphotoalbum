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

import KPhotoAlbum 1.0
import QtQuick 2.0
import QtQuick.Controls 1.1

ListView {
    id: root
    model: _remoteInterface.activeThumbnailModel
    snapMode: ListView.SnapToItem
    orientation: ListView.Horizontal
    flickDeceleration: 20000
    highlightMoveDuration: 200
    highlightRangeMode: ListView.StrictlyEnforceRange
    interactive: currentItem && currentItem.isZoomedOut

    Keys.onMenuPressed: menu.popup()
    Keys.onTabPressed: menu.popup() /* on desktop */
    Keys.onEscapePressed: {
        menu.visible = false
        keyboard.visible = false
        details.hide()
    }

    delegate: Zoomable {
        id: zoomable
        clip: true
        width: root.width
        height: root.height
        fitOnScreen: true
        property int imageId : model.imageId

        sourceComponent: Item {
            property QtObject sourceSize : QtObject {
                readonly property int width: remoteImage.width
                readonly property int height: remoteImage.height
            }

            RemoteImage {
                id: remoteImage
                scale: parent.width / width
                transformOrigin: Item.TopLeft
                imageId: model.imageId
                type: Enums.Images

                Connections {
                    target: zoomable
                    onZoomStarted: remoteImage.loadFullSize()
                }
            }
        }
    }

    MouseArea {
        z: -1
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: parent.height/5
        onClicked: details.show()
    }

    MouseArea {
        z: -1
        anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
        width: parent.width/5
        onClicked: root.decrementCurrentIndex()
    }

    MouseArea {
        z: -1
        anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
        width: parent.width/5
        onClicked: root.incrementCurrentIndex()
    }

    MouseArea {
        z: -1
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: parent.height/5
        onClicked: keyboard.visible = true
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

    MouseArea {
        enabled: keyboard.visible || details.visible
        anchors.fill: parent
        onClicked: {
            keyboard.visible = false
            details.hide()
        }
    }

    Keyboard {
        id: keyboard
        anchors.centerIn: parent
        visible: false
        onVisibleChanged: _remoteInterface.requestDetails(root.currentItem.imageId)
        onLetterSelected: _remoteInterface.setToken(root.currentItem.imageId, letter)
        onLetterDeselected: _remoteInterface.removeToken(root.currentItem.imageId, letter)
        selected: _remoteInterface.tokens
    }

    ImageDetails {
        id: details
        anchors.centerIn: parent
        imageId: currentItem ? currentItem.imageId : -1
    }

    Menu {
        id: menu
        title: "Context Menu"
        MenuItem {
            text: "Image details"
            onTriggered: details.show()
        }
        MenuItem {
            text: "Add/Remove tokens..."
            onTriggered: keyboard.visible = true
        }
        MenuItem {
            text: "Refine search"
            onTriggered: _remoteInterface.showOverviewPage()
        }
        MenuItem {
            text: "Go Home"
            onTriggered: _remoteInterface.goHome()
        }
    }

    onCurrentIndexChanged: {
        if (keyboard.visible)
            _remoteInterface.requestDetails(root.currentItem.imageId)
    }
}

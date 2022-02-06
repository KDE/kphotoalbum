/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import KPhotoAlbum 1.0
import QtQuick 2.15
import QtQuick.Controls 2.15 // MenuItem

Item {
    ListView {
        id: listview
        property bool isZoomedOut: true

        anchors.fill: parent

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
            _imageDetails.visible = false
        }

        Connections {
            target: _slideShow
            function onRequestNext() { listview.incrementCurrentIndex() }
        }

        delegate: Zoomable {
            id: zoomable
            clip: true
            width: listview.width
            height: listview.height
            fitOnScreen: true
            property int imageId : model.imageId

            // The PressAndHoldArea below can't see this item so communicate view the listview.
            onIsZoomedOutChanged: listview.isZoomedOut = isZoomedOut


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
                        function onZoomStarted() {
                            remoteImage.loadFullSize()
                        }
                    }
                }
            }
        }

        MouseArea {
            z: -1
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: parent.height/5
            onClicked: _imageDetails.visible = true
        }

        MouseArea {
            z: -1
            anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
            width: parent.width/5
            onClicked: listview.decrementCurrentIndex()
        }

        MouseArea {
            z: -1
            anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
            width: parent.width/5
            onClicked: listview.incrementCurrentIndex()
        }

        MouseArea {
            z: -1
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: parent.height/5
            onClicked: keyboard.visible = true
        }

        Connections {
            target: _remoteInterface
            function onJumpToImage(index) {
                var tmp = listview.highlightMoveDuration;
                listview.highlightMoveDuration = 0
                listview.currentIndex = index
                listview.highlightMoveDuration = tmp
            }
        }

        MouseArea {
            enabled: keyboard.visible || _imageDetails.visible
            anchors.fill: parent
            onClicked: {
                keyboard.visible = false
                _imageDetails.visible = false
            }
        }

        Keyboard {
            id: keyboard
            anchors.centerIn: parent
            visible: false
            onVisibleChanged: _remoteInterface.requestDetails(listview.currentItem.imageId)
            onLetterSelected: _remoteInterface.setToken(listview.currentItem.imageId, letter)
            onLetterDeselected: _remoteInterface.removeToken(listview.currentItem.imageId, letter)
            selected: _remoteInterface.tokens
        }

        ImageDetails {
            id: details
            anchors.centerIn: parent
            imageId: listview.currentItem ? listview.currentItem.imageId : -1
            visible: _imageDetails.visible
        }

        Menu { // FIXME delete
            id: menu
            title: "Context Menu"
            MenuItem {
                text: "Image details"
                onTriggered: _imageDetails.visible = true
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

        PressAndHoldArea {
            enabled: !_imageDetails.visible && !keyboard.visible && listview.isZoomedOut
            anchors.fill: parent
            onPressAndHold: {
                contextMenu.popup(mouseX, mouseY)
            }
        }

        onCurrentIndexChanged: {
            _imageDetails.visible = false
            if (keyboard.visible)
                _remoteInterface.requestDetails(listview.currentItem.imageId)
        }
        onVisibleChanged: {
            if (!visible)
                _slideShow.running = false
        }
    }
    ContextMenu {
        id: contextMenu
        imageViewer: true
        imageId: listview.currentItem ? listview.currentItem.imageId : -1
        onRequestAdjustSpeed: slider.visible = true
    }

    SlideShowSlider {
        id: slider
        anchors.fill: parent
        visible: false
    }
}

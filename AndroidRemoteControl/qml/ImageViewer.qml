/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import KPhotoAlbum 1.0
import QtQuick 2.15
import QtQuick.Controls 2.15 // MenuItem

Item {
    id: root
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
            readonly property bool isCurrentItem : ListView.isCurrentItem

            // The PressAndHoldArea below can't see this item so communicate view the listview.
            onIsZoomedOutChanged: listview.isZoomedOut = isZoomedOut


            sourceComponent: Item {
                property QtObject sourceSize : QtObject {
                    readonly property int width: model.isVideo ? _screenInfo.viewWidth :remoteImage.width
                    readonly property int height: model.isVideo ? _screenInfo.viewHeight : remoteImage.height
                }

                RemoteImage {
                    id: remoteImage
                    scale: parent.width / width
                    transformOrigin: Item.TopLeft
                    imageId: model.isVideo ? -1 : model.imageId
                    type: Enums.Images
                    visible: !model.isVideo

                    Connections {
                        target: zoomable
                        function onZoomStarted() {
                            if (!model.isVideo)
                                remoteImage.loadFullSize()
                        }
                    }
                }
                VideoViewer {
                    anchors.centerIn: parent
                    scale: {
                        if (!listview.isZoomedOut)
                            return parent.width / width
                        var widthScale = root.width / width
                        var heightScale = root.height / height
                        return Math.min(Math.min(widthScale, heightScale), 1)
                    }
                    active: model.isVideo && zoomable.isCurrentItem && root.visible
                    imageId: model.isVideo ? model.imageId : -1
                }
            }
        }

        MouseArea {
            id: bottomArea
            z: -1
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: parent.height/5
            onClicked: _imageDetails.visible = true
        }
        MouseArea {
            id: leftArea
            z: -1
            anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
            width: parent.width/5
            onClicked: listview.decrementCurrentIndex()
        }
        MouseArea {
            id: rightArea
            z: -1
            anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
            width: parent.width/5
            onClicked: listview.incrementCurrentIndex()
        }
        MouseArea {
            id: topArea
            z: -1
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: parent.height/5
            onClicked: keyboard.visible = true
        }
        MouseArea {
            id: cancelingArea
            enabled: keyboard.visible || _imageDetails.visible
            anchors.fill: parent
            onClicked: {
                keyboard.visible = false
                _imageDetails.visible = false
            }
        }
        PressAndHoldArea {
            id: contextMenuHandler
            enabled: !_imageDetails.visible && !keyboard.visible && listview.isZoomedOut
            anchors.fill: parent
            // This cannot go over the slider used when playing videos, otherwise it will steal the videos events.
            anchors.bottomMargin: _screenInfo.pixelForSizeInMM(15).height
            onPressAndHold: {
                contextMenu.popup(mouseX, mouseY)
            }
        }
        Connections { // connect onJumpToImage from C++
            target: _remoteInterface
            function onJumpToImage(index) {
                var tmp = listview.highlightMoveDuration;
                listview.highlightMoveDuration = 0
                listview.currentIndex = index
                listview.highlightMoveDuration = tmp
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

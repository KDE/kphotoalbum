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

ListView {
    id: root
    model: _remoteInterface.activeThumbnailModel
    snapMode: ListView.SnapToItem
    orientation: ListView.Horizontal
    flickDeceleration: 20000
    highlightMoveDuration: 200
    highlightRangeMode: ListView.StrictlyEnforceRange
    interactive: currentItem && currentItem.isZoomedOut

//    ImageDetails {
//        id: details
//        anchors.centerIn: parent
//        imageId: currentItem ? currentItem.imageId : -1
//    }

    delegate: Zoomable {
        width: root.width
        height: root.height
        fitOnScreen: true
        //property alias imageId : remoteImage.imageId
        sourceComponent: Item {
            id: delegateRoot
            property QtObject sourceSize : QtObject {
                readonly property int width: remoteImage.width
                readonly property int height: remoteImage.height
            }

            RemoteImage {
                id: remoteImage
                scale: delegateRoot.width / width
                transformOrigin: Item.TopLeft
                imageId: model.imageId
                type: Enums.Images
            }
        }
    }

//        Flickable {
//        width: root.width
//        height: root.height
//        contentWidth: root.width
//        contentHeight: root.height
//        onContentWidthChanged: console.log("ContentsWidth: "  + contentWidth)

//        id: flick
//        property alias imageId : remoteImage.imageId
//        RemoteImage {
//            id: remoteImage
//            width: root.width
//            height: root.height
//            imageId: model.imageId
//            type: Enums.Images
//            PinchArea {
//                width: Math.max(flick.contentWidth, flick.width)
//                height: Math.max(flick.contentHeight, flick.height)

//                property real initialWidth
//                property real initialHeight
//                onInitialWidthChanged: console.log("initialWithd: " + initialWidth)

//                //pinch.target: parent
////                anchors.fill: parent
//                pinch.minimumScale: 1
//                pinch.maximumScale: 10
//                pinch.dragAxis: Pinch.XandYAxis

//                onPinchStarted: {
//                    initialWidth = flick.contentWidth
//                    initialHeight = flick.contentHeight
//                }

//                onPinchUpdated: {
//                    flick.contentX += pinch.previousCenter.x - pinch.center.x
//                    flick.contentY += pinch.previousCenter.y - pinch.center.y
//                    flick.resizeContent(initialWidth * pinch.scale, initialHeight * pinch.scale, pinch.center)
//                }
//                MouseArea {
//                    anchors.fill: parent
//                    onClicked: {
//                        if ( mouse.x < root.width/2 )
//                            root.decrementCurrentIndex()
//                        else
//                            root.incrementCurrentIndex()
//                    }
//                    onPressAndHold: details.show()
//                }
//            }
//        }
//    }

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

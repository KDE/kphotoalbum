/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import KPhotoAlbum 1.0
import QtQuick.Controls 1.1
import QtQuick.Extras 1.4

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

    Keys.onMenuPressed: menu.popup()
    Keys.onTabPressed: menu.popup() /* on desktop */
    Keys.onEscapePressed: menu.visible = false

    GridView {
        id: grid
        anchors.fill: parent
        transformOrigin: Qt.TopLeftCorner

        cellWidth: imageWidth() + padding()
        cellHeight: imageWidth() + padding() + (root.showLabels ? 30 : 0)

        delegate: Item {
            Column {
                x: (padding + grid.cellWidth - width)/2
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
                        onPressAndHold: menu.popup() // FIXME
                    }
                }
                Text {
                    visible: root.showLabels
                    color: _settings.textColor
                    anchors { left: parent.left; right: parent.right; margins: padding/2 }
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

    PositionObserver {
        id: observer
        view: grid
    }

    PieMenu {
        id: pieMenu
        triggerMode: TriggerMode.TriggerOnRelease

        MenuItem {
            text: "Run Slide Show"
            iconSource: "qrc:/Images/view-presentation.png"
            onTriggered: console.log("Run SlideShow")
        }

//        MenuItem {
//            text: "Show Time Line"
//            onTriggered: console.log("Show timeline")
//        }

        MenuItem {
            text: "Refine Search"
            iconSource: "qrc:/Images/system-search.png"
            onTriggered: _remoteInterface.showOverviewPage()
        }

        MenuItem {
            text: "Go Home"
            iconSource: pieMenu.visible ? "image://images/home" : ""
            onTriggered: _remoteInterface.goHome()
        }


    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true
        onPressAndHold: pieMenu.popup(mouseX, mouseY)
    }


    function imageWidth() {
        return type == Enums.CategoryItems ? _settings.categoryItemSize : _settings.thumbnailSize;
    }
    function padding() {
        return 20 // imageWidth()/8 - rhk would like us to try not to scale the space.
    }
}

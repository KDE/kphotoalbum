/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import KPhotoAlbum 1.0

PinchArea {
    id: root
    anchors.fill: parent
    pinch.minimumScale: 0.1
    pinch.maximumScale: 10

    onPinchStarted: {
        initialSize = _settings.overviewIconSize
    }

    onPinchUpdated: {
        iconScale =  1 + (pinch.scale-1)*0.25
        _settings.overviewIconSize = initialSize * iconScale
    }

    onPinchFinished: {
        iconScale = 1
        _remoteInterface.rerequestOverviewPageData()
    }

    property double iconScale : 1
    property double initialSize

    Flickable {
        id: flickable
        anchors.fill: parent
        contentWidth: grid.width
        contentHeight: grid.height
        flickableDirection: Flickable.VerticalFlick

        Grid {
            id: grid
            x: Math.max(0, (flickable.width - width) /2)
            y: Math.max(0, (flickable.height - height)/2)
            columns: _screenInfo.overviewColumnCount
            spacing: _screenInfo.overviewSpacing

            Icon {
                text: "home"
                icon: _images.home
                width: _screenInfo.overviewIconSize
                iconScale: root.iconScale
                onClicked: _remoteInterface.goHome()
            }

            Repeater {
                model: _remoteInterface.categories
                delegate: Icon {
                    enabled: model.enabled
                    text: model.name
                    icon: model.icon
                    width: _screenInfo.overviewIconSize
                    iconScale: root.iconScale
                    onClicked: _remoteInterface.selectCategory(model.name, model.type)
                }
            }

            Icon {
                text: "Discover"
                icon: _images.discovery
                width: _screenInfo.overviewIconSize
                iconScale: root.iconScale
                onClicked: _remoteInterface.doDiscovery()
            }

            Icon {
                text: "View"
                icon: _images.kphotoalbum
                width: _screenInfo.overviewIconSize
                iconScale: root.iconScale
                onClicked: _remoteInterface.showThumbnails()
            }
        }
    }
    Rectangle {
        anchors.fill: parent
        color: _settings.backgroundColor
        opacity: _remoteInterface.categories.hasData ? 0 : 1
        visible: opacity != 0
        Behavior on opacity { NumberAnimation { duration: 300 } }
    }
}


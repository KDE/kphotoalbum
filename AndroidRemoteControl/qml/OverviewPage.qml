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

Flickable {
    id: root

    contentWidth: grid.width
    contentHeight: grid.height
    flickableDirection: Flickable.VerticalFlick

    Grid {
        id: grid
        x: Math.max(0, (root.width - width) /2)
        y: Math.max(0, (root.height - height)/2)
        columns: _screenInfo.overviewColumnCount
        spacing: _screenInfo.overviewSpacing

        Icon {
            text: "home"
            icon: _remoteInterface.home
            width: _screenInfo.overviewIconSize
            onClicked: _remoteInterface.goHome()
        }

        Repeater {
            model: _remoteInterface.categories
            delegate: Icon {
                //enabled: model.enabled
                text: model.text
                icon: model.icon
                width: _screenInfo.overviewIconSize
                onClicked: _remoteInterface.selectCategory(model.name, model.type)
            }
        }

        Icon {
            text: "Discover"
            icon: _remoteInterface.discoveryImage
            width: _screenInfo.overviewIconSize
            onClicked: _remoteInterface.doDiscovery()
        }

        Icon {
            text: "View"
            icon: _remoteInterface.kphotoalbum
            width: _screenInfo.overviewIconSize
            onClicked: _remoteInterface.showThumbnails()
        }
    }
}

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

Item {
    id: root
    property string text
    property variant icon
    property double iconScale
    signal clicked()

    height: column.height

    Column {
        id: column
        anchors {left: parent.left; right: parent.right}
        spacing: 10 // Value used in ScreenInfo::iconHeight, so update there too if changing the value.
        MyImage {
            image: root.icon
            scale: iconScale
            width: imageWidth * scale
            height: imageHeight * scale
            transformOrigin: Item.TopLeft
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: root.text
            color: _settings.textColor
            clip: true
            anchors { left: parent.left; right: parent.right }
            horizontalAlignment: Text.AlignHCenter
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}

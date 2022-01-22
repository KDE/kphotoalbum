/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
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

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

Rectangle {
    id: root
    property alias text : text.text
    property bool isSelected: false

    signal selected()
    signal deselected()
    signal clicked()

    color: isSelected ? "#DD000000" : "#DD888888"
    border { color: "black"; width: 2}
    Text {
        id: text
        anchors.centerIn: parent
        color: isSelected ? "white" : "black"
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (isSelected)
                /*emit*/ root.deselected()
            else
                /*emit*/ root.selected()
            /*emit*/ root.clicked()
        }
    }
}

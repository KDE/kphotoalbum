/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

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

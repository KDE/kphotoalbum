/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

ListView {
    id: root
    signal clicked(string label)

    delegate: Text {
        font.pointSize: 30
        anchors {left: parent.left; right: parent.right}
        text: (modelData == "**NONE**") ? "None" : modelData
        color: _settings.textColor
        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked(modelData)
        }
    }

    ScrollBar {
        flickable: root
    }

    PositionObserver {
        objectName: "listViewPageObserver"
        view: parent
    }
}

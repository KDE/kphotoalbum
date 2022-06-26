/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
ListView {
    id: root
    signal clicked(string label)

    delegate: Text {
        font.pointSize: 30
        width: root.width
        text: (modelData === "**NONE**") ? "None" : modelData
        readonly property string searchLabel : text[0]
        color: _settings.textColor

        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked(modelData)
        }
    }

    QQC2.ScrollBar.vertical: QQC2.ScrollBar {
        id: scrollbar
        width: _screenInfo.dotsPerMM * 5
    }

    SearchBarIndicator {
        scrollbar : scrollbar
        view: root
    }


    PositionObserver {
        objectName: "listViewPageObserver"
        view: parent
    }
}

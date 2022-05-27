/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Rectangle {
    property int milliseconds
    color: Qt.rgba(1,1,1,0.5)
    implicitWidth: currentTime.implicitWidth + _screenInfo.pixelForSizeInMM(2).width
    implicitHeight: currentTime.implicitHeight
    Text {
        id: currentTime
        anchors.centerIn: parent
        text: {
            var seconds = milliseconds/1000
            var minutes = Math.floor(seconds/60);
            var secs = Math.floor(seconds%60)
            return minutes + ":" + (secs < 10 ? "0" : "") + secs
        }
    }
}

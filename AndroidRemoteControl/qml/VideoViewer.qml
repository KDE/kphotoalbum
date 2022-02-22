/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtMultimedia 5.0
import KPhotoAlbum 1.0

VideoOutput {
    id: root
    property alias imageId: remoteVideo.imageId
    property alias active : remoteVideo.active
    source: media
    RemoteVideoInfo {
        id: remoteVideo
    }

    MediaPlayer {
        id: media
        source: root.active ? remoteVideo.url : ""
        autoPlay: true
        onError: console.log("Error from Media Player:" + errorString)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: media.play()
    }
}

/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtMultimedia 5.15
import KPhotoAlbum 1.0
import QtQuick.Controls 2.15

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
        notifyInterval: 250
        onError: console.log("Error from Media Player:" + errorString)
        onPlaying: _slideShow.videoRunning = true
        onStopped: _slideShow.videoRunning = false
    }

    MouseArea {
        anchors.centerIn: parent
        width: parent.width * 2/5
        height: parent.height * 2/5

        onClicked: {
            if (media.playbackState === MediaPlayer.PlayingState)
                media.pause()
            else
                media.play()
        }
    }

    ProgressBar {
        anchors.centerIn: parent
        value: remoteVideo.progress
        visible: root.active && value != 1
    }
    Slider {
        // FIXME when portait image is shown in landscape, the slider is scaled down in size.
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom

            leftMargin: _screenInfo.pixelForSizeInMM(5).width
            rightMargin: _screenInfo.pixelForSizeInMM(5).width
            bottomMargin: _screenInfo.pixelForSizeInMM(5).height
        }
        // FIXME there is a few milliseconds where the handle is in the middle of the screen
        visible: media.duration > 0
        handle.height: implicitHandleHeight * 2

        to: media.duration
        value: media.position

        onMoved: media.seek(value)
    }
}

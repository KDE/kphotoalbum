/* SPDX-FileCopyrightText: 2022 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 1.4 as QQC1 // We need Menu from QQC 1, not 2!
import QtQuick.Extras 1.4

PieMenu {
    id: root
    property int imageId : -1
    property bool imageViewer : false
    signal requestAdjustSpeed()

    triggerMode: TriggerMode.TriggerOnRelease

    QQC1.MenuItem {
        text: "Run Slide Show"
        visible: !_slideShow.running
        iconSource: _images.ready ? "image://images/slideShow" : ""
        onTriggered: {
            _slideShow.running = true
            if (!imageViewer)
                _remoteInterface.showImage(imageId)
        }
    }

    QQC1.MenuItem {
        text: "Stop Slide Show"
        visible: _slideShow.running
        iconSource: _images.ready ? "image://images/stopSlideShow" : ""
        onTriggered: _slideShow.running = false
    }

//        QQC1.MenuItem {
//            text: "Show Time Line"
//            onTriggered: console.log("Show timeline")
//        }

    QQC1.MenuItem {
        visible: !imageViewer
        text: "Refine Search"
        iconSource: _images.ready ? "image://images/search" : ""
        onTriggered: _remoteInterface.showOverviewPage()
    }

    QQC1.MenuItem {
        text: "Go Home"
        iconSource: _images.ready ? "image://images/home" : ""
        onTriggered: _remoteInterface.goHome()
    }

    QQC1.MenuItem {
        visible: imageViewer
        text: "Image Details"
        iconSource: _images.ready ? "image://images/info" : ""
        onTriggered: _imageDetails.visible = true
    }

    QQC1.MenuItem {
        visible: root.imageId != -1
        text: "Jump to Context"
        iconSource: _images.ready ? "image://images/kphotoalbum" : ""
        onTriggered: _remoteInterface.jumpToContext(root.imageId)
    }

    QQC1.MenuItem {
        visible: imageViewer && _slideShow.running
        text: "Adjust Speed"
        iconSource: _images.ready ? "image://images/slideShowSpeed" : ""
        onTriggered: root.requestAdjustSpeed()
    }

}

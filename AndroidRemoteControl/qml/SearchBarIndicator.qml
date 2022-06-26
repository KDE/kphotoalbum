/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

// This is the white box next to scrollbar showing how far we have scrolled down.
// It requires that the items being scrolled has a property named 'searchLabel'
Rectangle {
    property Item scrollbar
    property Item view

    width: text.implicitWidth + 30
    height: text.implicitHeight + 10
    color: "white"
    anchors.right: scrollbar.left
    y: (scrollbar.height-scrollbar.width) * scrollbar.position
    visible: topSearchLabel() !== null
    opacity: scrollbar.contentItem.opacity > 0.5 ? 1 : scrollbar.contentItem.opacity

    Text {
        id: text
        anchors.centerIn: parent
        text: parent.topSearchLabel()
    }

    function topSearchLabel() {
        var item = view.itemAt(view.contentX, view.contentY)
        return item ? item.searchLabel : null
    }
}

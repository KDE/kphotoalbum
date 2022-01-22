/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

Item {
    property var view
    property int index: 0

    Connections {
        target: view
        function onCountChanged() { scrollToStoredOffset() }
        function onVisibleChanged() { scrollToStoredOffset() }
    }

    function scrollToStoredOffset() {
        view.positionViewAtIndex(index, GridView.Beginning)
    }

    function getIndex() {
        return view.indexAt(view.contentX, view.contentY)
    }
}

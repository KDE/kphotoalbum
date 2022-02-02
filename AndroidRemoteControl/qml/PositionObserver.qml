/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

// The purpose of this item is to scroll the listview from the C++ side.
// The are two situations where we need to do that:
// 1) When using jump to context.
// 2) When going back in history, in which case we want to go to the area of the list view we were at when going forward.
// It is unfortunately not enough to just scroll when the index property is changed (from the C++ site), as the view might not have loaded yet
Item {
    property var view
    property int index: 0

    onIndexChanged: scrollToStoredOffset()
    Connections {
        target: view
        function onCountChanged() { scrollToStoredOffset() }
        // This doesn't seem to be needed, but lets keep it arround for a while
        // function onVisibleChanged() { scrollToStoredOffset() }
    }

    function scrollToStoredOffset() {
        view.positionViewAtIndex(index, GridView.Beginning)
    }

    function getIndex() {
        return view.indexAt(view.contentX, view.contentY)
    }
}

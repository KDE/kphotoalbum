/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

Item {
    id: root

    signal letterSelected(string letter)
    signal letterDeselected(string letter)
    property var selected

    width: childrenRect.width
    height: childrenRect.height
    opacity: visible ? 1 : 0
    Behavior on opacity { NumberAnimation { duration: 200 } }

    Cell {
        anchors {
            left: grid.left
            right: grid.right
            bottom: grid.top
        }
        height: cellSize()
        text: "Select Token:"
    }

    Grid {
        id: grid
        columns: 5
        Repeater {
            model: ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"]

            Cell {
                width: cellSize()
                height: cellSize()
                text: modelData
                isSelected: hasKey(root.selected, modelData)
                onSelected: {
                    letterSelected(modelData)
                    root.visible = false
                }
                onDeselected: {
                    letterDeselected(modelData)
                    root.visible = false
                }
            }
        }
    }

    Cell {
        anchors { right: grid.right; bottom: grid.bottom }
        height: cellSize()
        width: 4 * cellSize()
        text: "Cancel"
        onClicked: root.visible = false
    }


    function cellSize() {
        return Math.min(_screenInfo.viewWidth, _screenInfo.viewHeight) / 10
    }

    function hasKey(array, key) {
        for (var i = 0; i < array.length; i++) {
            if (array[i] === key) {
                return true;
            }
        }
        return false;
    }
}

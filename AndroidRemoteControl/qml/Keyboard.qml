import QtQuick 2.0

Item {
    id: root

    signal letterSelected(string letter)

    width: grid.width
    height: grid.height

    Grid {
        id: grid
        columns: 5
        Repeater {
            model: ["A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"]

            Cell {
                width: cellSize()
                height: cellSize()
                text: modelData
                onClicked: {
                    letterSelected(modelData)
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
}

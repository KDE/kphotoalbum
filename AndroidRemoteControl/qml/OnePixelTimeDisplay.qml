import QtQuick 2.0

Item {
    property int current : 0
    property int max : 0

    Rectangle {
        width: max == 0 ? 0 : parent.width * current / max
        height: parent.height
        x: 0
        y: 0
        color: "grey"
    }
}

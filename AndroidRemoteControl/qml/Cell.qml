import QtQuick 2.0

Rectangle {
    id: root
    property alias text : text.text
    signal clicked()
    color: "#AA000000"
    border { color: "black"; width: 2}
    Text {
        id: text
        anchors.centerIn: parent
        color: "white"
    }
    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}

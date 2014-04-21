import QtQuick 2.0

ListView {
    id: root
    signal clicked(string label)

    delegate: Text {
        font.pointSize: 30
        anchors {left: parent.left; right: parent.right}
        text: (modelData == "**NONE**") ? "None" : modelData
        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked(modelData)
        }
    }

    ScrollBar {
        flickable: root
    }
}

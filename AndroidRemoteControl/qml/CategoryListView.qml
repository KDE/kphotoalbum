import QtQuick 2.0

ListView {
    id: root
    signal clicked(int imageId)

    delegate: Text {
        anchors {left: parent.left; right: parent.right}
        text: modelData
        MouseArea {
            anchors.fill: parent
            onClicked: root.clicked(modelData)
        }
    }
}

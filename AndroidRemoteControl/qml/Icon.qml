import QtQuick 2.0
import KPhotoAlbum 1.0

Item {
    id: root
    property string text
    property variant icon
    signal clicked()

    height: column.height

    Column {
        id: column
        anchors {left: parent.left; right: parent.right}
        spacing: 10
        MyImage {
            image: root.icon
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: root.text
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}

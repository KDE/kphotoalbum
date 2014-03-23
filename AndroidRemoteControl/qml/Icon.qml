import QtQuick 2.0
import KPhotoAlbum 1.0

Item {
    id: root
    property string text
    property variant icon
    signal clicked()

    width: 200
    height: 150

    Column {
        anchors.centerIn: parent
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

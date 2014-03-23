import QtQuick 2.0
import KPhotoAlbum 1.0

Item {
    GridView {
        Rectangle {
            anchors.fill: parent
            color: "#DDDDDD"
            z: -1
        }

        anchors.centerIn: parent
        width: 450
        height: 400

        cellWidth: 150
        cellHeight: 200

        model: _remoteInterface.categories

//        Item {
//            width: parent.cellWidth
//            height: parent.cellHeight
//            Column {
//                anchors.centerIn: parent
//                MyImage { image: _remoteInterface.home }
//                Text { text: "home" }
//            }
//        }

        delegate: Item {
            width: GridView.view.cellWidth
            height: GridView.view.cellHeight

            Column {
                anchors.centerIn: parent
                MyImage { image: model.icon }
                Text { text: model.text }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: console.log(model.name)
            }

        }
    }
}

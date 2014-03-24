import QtQuick 2.0
import KPhotoAlbum 1.0

Item {
//    Rectangle {
//        anchors.fill: grid
//        color: "#DDDDDD"
//    }

    Grid {
        id: grid
        anchors.centerIn: parent
        width: 200*3
        height: 150*3
        columns: 3

        Icon {
            text: "home"
            icon: _remoteInterface.home
            onClicked: _remoteInterface.goHome()
        }

        Repeater {
            model: _remoteInterface.categories
            delegate: Icon {
                text: model.text
                icon: model.icon
                onClicked: _remoteInterface.selectCategory(model.name)
            }
        }

        Icon {
            text: "Show Thumbnails"
            icon: _remoteInterface.kphotoalbum
            onClicked: _remoteInterface.showThumbnails()
        }
    }
}

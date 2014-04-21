import QtQuick 2.0
import KPhotoAlbum 1.0

Flickable {
    id: root

    contentWidth: grid.width
    contentHeight: grid.height
    flickableDirection: Flickable.VerticalFlick

    Binding {
        target: _screenInfo
        property: "overviewScreenWidth"
        value: root.width
    }

    Grid {
        id: grid
        x: Math.max(0, (root.width - width) /2)
        y: Math.max(0, (root.height - height)/2)
        columns: _screenInfo.overviewColumnCount
        spacing: _screenInfo.overviewSpacing

        Icon {
            text: "home"
            icon: _remoteInterface.home
            width: _screenInfo.overviewIconSize
            onClicked: _remoteInterface.goHome()
        }

        Repeater {
            model: _remoteInterface.categories
            delegate: Icon {
                //enabled: model.enabled
                text: model.text
                icon: model.icon
                width: _screenInfo.overviewIconSize
                onClicked: _remoteInterface.selectCategory(model.name, model.type)
            }
        }

        Icon {
            text: "View"
            icon: _remoteInterface.kphotoalbum
            width: _screenInfo.overviewIconSize
            onClicked: _remoteInterface.showThumbnails()
        }
    }
}

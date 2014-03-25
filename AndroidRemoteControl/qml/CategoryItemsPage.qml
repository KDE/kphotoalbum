import QtQuick 2.0
import KPhotoAlbum 1.0

GridView {
    anchors.fill: parent
    cellWidth: 200
    cellHeight: 200

    model: _remoteInterface.categoryItemsModel
    delegate: Icon {
        text: model.text
        icon: model.icon
        onClicked: _remoteInterface.selectCategoryValue(model.text)
    }
}

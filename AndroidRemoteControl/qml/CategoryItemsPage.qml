import QtQuick 2.0
import KPhotoAlbum 1.0

PinchArea {
    pinch.minimumScale: 0.1
    pinch.maximumScale: 10
    onPinchUpdated: grid.scale = pinch.scale
    onPinchFinished: {
        _settings.categoryItemSize = _settings.categoryItemSize * pinch.scale
        grid.scale = 1
    }

    GridView {
        id: grid
        anchors.fill: parent
        cellWidth: _settings.categoryItemSize + 20
        cellHeight: _settings.categoryItemSize +  50

        model: _remoteInterface.categoryItemsModel
        delegate: Icon {
            width: _settings.categoryItemSize
            //height: _settings.categoryItemSize // HMMM WRONG!
            text: model.text
            icon: model.icon
            onClicked: _remoteInterface.selectCategoryValue(model.text)
        }
    }
}

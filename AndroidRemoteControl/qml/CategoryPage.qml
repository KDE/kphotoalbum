import QtQuick 2.0

ListView {
    model: _remoteInterface.categories
    delegate: Text { text: modelData }
}

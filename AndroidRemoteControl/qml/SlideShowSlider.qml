import QtQuick 2.0
import QtQuick.Controls 2.15

Item {
    id: root
    onVisibleChanged: {
        _slideShow.setOverride(!visible)
        if (visible)
            slider.value = _slideShow.interval
    }

    // Click outside of the slider show hide it
    MouseArea {
        anchors.fill: parent
        onClicked: root.visible = false
    }

    Slider {
        id: slider
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
            margins: 100
        }

        from: 1
        to: 120
        orientation: Qt.Vertical
        stepSize: 1
        snapMode: Slider.SnapAlways

        onPressedChanged:
            if (!pressed) {
                root.visible = false
                _slideShow.interval = slider.value
            }
    }

    Rectangle {
        anchors.centerIn: parent
        width: label.implicitWidth + 30
        height: label.implicitHeight + 30
        color: "white"

        Label {
            id: label
            anchors.centerIn: parent
            text: slider.value + " s"
            font.pointSize: 40
        }
    }
}

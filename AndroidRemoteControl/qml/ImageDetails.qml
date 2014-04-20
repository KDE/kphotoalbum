import QtQuick 2.0

Rectangle {
    property int imageId
    width: column.width + 40
    height: column.height + 40
    color: "#AA000000"
    visible: false

    onImageIdChanged: {
        if (visible)
            _remoteInterface.requestDetails(imageId)
    }

    MouseArea {
        anchors.fill: parent
        // Just eat all events, so a click outside a link doesn't go to next page
    }

    Column {
        id: column

        x: 20
        y: 20
        spacing: 15

        Text {
            text: "<b>Date</b>: " + _imageDetails.date
            color: "white"
        }
        Repeater {
            model: _imageDetails.categories
            Text {
                text: "<b>" + modelData + "</b>: " + items(modelData) + _imageDetails.dummy
                color: "white"
                linkColor: "white"
                onLinkActivated: { hide(); _remoteInterface.activateSearch(link) }
            }
        }
        Text {
            text: "<b>File name</b>: " + _imageDetails.fileName
            color: "white"
        }

        Text {
            text: "<b>Description</b>: " + _imageDetails.description.replace("\n","<br/>")
            color: "white"
            wrapMode: Text.Wrap
        }
    }

    Canvas {
        id:canvas
        anchors { right: parent.right; top: parent.top }
        width: 50
        height: 50
        onPaint: {
            var ctx = getContext("2d");
            ctx.fillStyle = "#FFFFFF";
            ctx.fillRect(0,0,width,height)
            ctx.lineWidth = 5
            ctx.moveTo(0,0)
            ctx.lineTo(width,height)
            ctx.moveTo(0,height)
            ctx.lineTo(width,0)
            ctx.stroke()
        }

        MouseArea {
            anchors.fill: parent
            onClicked: hide()
        }
    }

    Behavior on width {
        NumberAnimation {duration: 100 }
    }
    Behavior on height {
        NumberAnimation {duration: 100 }
    }

    function items(category) {
        var result
        var list = _imageDetails.itemsOfCategory(category)
        for (var i=0; i<list.length; ++i ) {
            var url=category + ";;;"+ list[i]
            var link = "<a href=\"" + url + "\">" + list[i] + "</a>"
            if (result)
                result = result + ", " + link
            else
                result = link
        }
        return result
    }

    function show() {
        _remoteInterface.requestDetails(imageId)
        visible = true
    }

    function hide() {
        visible = false
    }
}

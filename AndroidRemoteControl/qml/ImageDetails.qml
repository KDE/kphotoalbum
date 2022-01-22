/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

Item {
    id: root
    property int imageId

    visible: false
    opacity: visible ? 1 : 0
    Behavior on opacity { NumberAnimation { duration: 200 } }

    width: rect.width
    height: Math.min(_screenInfo.viewHeight, rect.height)
    anchors.centerIn: parent

    Flickable {
        contentWidth: rect.width
        contentHeight: rect.height
        anchors.fill: parent

        Rectangle {
            id: rect
            width: visible ? column.width + 40 : 0
            height: visible ? column.height + 40 : 0
            color: "#AA000000"

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
                        wrapMode: Text.Wrap
                        width: column.width
                    }
                }
                Text {
                    text: "<b>File name</b>: " + _imageDetails.fileName
                    color: "white"
                }

                Text {
                    visible: _imageDetails.description !== ""
                    width: Math.min(_screenInfo.viewWidth-200, implicitWidth)
                    textFormat: Text.RichText
                    text: "<b>Description</b>: " + _imageDetails.description.replace("\n","<br/>")
                    color: "white"
                    wrapMode: Text.Wrap
                }
            }

            Behavior on width {
                NumberAnimation {duration: 100 }
            }
            Behavior on height {
                NumberAnimation {duration: 100 }
            }

            SequentialAnimation {
                id: hideAnimation
                PropertyAnimation {
                    target: root
                    properties: "opacity"
                    to: 0
                    duration: 200
                }

                // I need to remove the visibility otherwise it will still steal all the event.
                PropertyAction {
                    target: root
                    property: "visible"
                    value: false
                }
            }
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
            x: -50; y: -50
            width: parent.width+100
            height: parent.height+100
            onClicked: hide()
        }
    }

    function items(category) {
        var result
        var list = _imageDetails.itemsOfCategory(category)
        for (var i=0; i<list.length; ++i ) {
            var url=category + ";;;"+ list[i]
            var link = "<a href=\"" + url + "\">" + list[i] + "</a>"
            var age = _imageDetails.age(category, list[i])
            if (age)
                link += age
            if (result)
                result = result + ", " + link
            else
                result = link
        }
        return result
    }

    function hide() {
        hideAnimation.restart()
    }

    function show() {
        _remoteInterface.requestDetails(imageId)
        visible = true
    }
}

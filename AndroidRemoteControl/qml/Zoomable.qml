/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0

Item {
    id: root

    // Should the image be fitted on the screen intially (so all of the image can be seen), or should we use all the screen space instead?
    property bool fitOnScreen: false
    property alias sourceComponent : target.sourceComponent
    property bool isZoomedOut: flick.contentWidth <= initialWidth() && flick.contentHeight <= initialHeight()

    signal zoomStarted()

    readonly property bool inPortraitMode: width < height

    Flickable {
        id: flick
        anchors.fill: parent

        contentX: initialX()
        contentY: initialY()
        contentWidth: initialWidth()
        contentHeight: initialHeight()
        leftMargin: -initialX()
        topMargin: -initialY()
        interactive: !isZoomedOut

        Loader {
            id: target
            anchors.fill: flick.contentItem

            PinchArea {
                id:pinchArea
                width: Math.max(flick.contentWidth, flick.width)
                height: Math.max(flick.contentHeight, flick.height)

                property real initialZoomWidth
                property real initialZoomHeight

                onPinchStarted: {
                    initialZoomWidth = flick.contentWidth
                    initialZoomHeight = flick.contentHeight

                    flick.contentWidth = Qt.binding(function() {return flick.contentItem.width})
                    flick.contentHeight = Qt.binding(function() {return flick.contentItem.height})

                    // We need to disable the Flickable, otherwise it will move the item as soon as the first finger is released.
                    flick.interactive = false
                    root.zoomStarted()
                }

                onPinchUpdated: {
                    flick.contentX += pinch.previousCenter.x - pinch.center.x
                    flick.contentY += pinch.previousCenter.y - pinch.center.y

                    var width = initialZoomWidth * pinch.scale
                    var height = initialZoomHeight * pinch.scale
                    if (width < initialWidth())
                        width = initialWidth()
                    if (height < initialHeight())
                        height = initialHeight()
                    flick.resizeContent(width, height, pinch.center)
                }

                onPinchFinished: {
                    flick.interactive = Qt.binding(function() { return !isZoomedOut })
                    flick.returnToBounds()
                }


                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: returnToFullScreen()
                    propagateComposedEvents: true
                }
            }
        }
    }

    function returnToFullScreen() {
        // If these were just assignments, the centering of the image would not work when the device was rotated.
        flick.contentWidth = Qt.binding(function() { return initialWidth() })
        flick.contentHeight = Qt.binding(function() { return initialHeight() })
        flick.contentX = Qt.binding( function() { return initialX() })
        flick.contentY = Qt.binding( function() { return initialY() })
        flick.leftMargin = Qt.binding(function() { return -initialX() })
        flick.topMargin = Qt.binding(function() { return -initialY() })
    }

    function aspect() {
        return  target.item.sourceSize.width / target.item.sourceSize.height
    }

    // Scale factor that will make the image fit on the screen
    function fitScale() {
        return Math.min(root.width/target.item.sourceSize.width, root.height/target.item.sourceSize.height)
    }

    function initialWidth() {
        if (root.fitOnScreen)
            return target.item.sourceSize.width * fitScale()
        else
            return inPortraitMode ? root.height * aspect() : root.width
    }

    function initialHeight() {
        if (root.fitOnScreen)
            return target.item.sourceSize.height * fitScale()
        else
            return inPortraitMode ? root.height : root.width / aspect()
    }

    function initialX() {
        return flick.contentWidth > root.width ? 0 : -(root.width-flick.contentWidth)/2
    }

    function initialY() {
        return flick.contentHeight > root.height ? 0 : -(root.height-flick.contentHeight)/2
    }
}

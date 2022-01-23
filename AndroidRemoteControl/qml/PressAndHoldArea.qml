import QtQuick 2.15

// The seems to be a bug in Qt relating to using a Flickable and a MouseArea with pressAndHold
// See https://stackoverflow.com/questions/29236762/mousearea-inside-flickable-is-preventing-it-from-flicking
// This is a work around for this.
MouseArea {
    propagateComposedEvents: false
    onReleased: propagateComposedEvents = true
}

#ifndef REMOTECONTROL_POSITIONOBSERVER_H
#define REMOTECONTROL_POSITIONOBSERVER_H

class QQuickView;

namespace RemoteControl {

class PositionObserver
{
public:
    static void setView(QQuickView* view);
    static void setThumbnailOffset(int index);
    static int thumbnailOffset();
};

} // namespace RemoteControl

#endif // REMOTECONTROL_POSITIONOBSERVER_H

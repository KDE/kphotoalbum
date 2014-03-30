#ifndef REMOTECONTROL_SETTINGS_H
#define REMOTECONTROL_SETTINGS_H

#include <QObject>

namespace RemoteControl {

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double thumbnailSize READ thumbnailSize WRITE setThumbnailSize NOTIFY thumbnailSizeChanged)

    double m_thumbnailScale;

public:
    static Settings& instance();
    int thumbnailSize() const;
    void setThumbnailSize(int size);

signals:
    void thumbnailSizeChanged();

private:
    explicit Settings() = default;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SETTINGS_H

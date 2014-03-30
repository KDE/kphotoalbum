#ifndef REMOTECONTROL_SETTINGS_H
#define REMOTECONTROL_SETTINGS_H

#include <QObject>

namespace RemoteControl {

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double thumbnailScale READ thumbnailScale WRITE setThumbnailScale NOTIFY thumbnailScaleChanged)

    double m_thumbnailScale;

public:
    static Settings& instance();
    double thumbnailScale() const;
    void setThumbnailScale(double arg);

signals:
    void thumbnailScaleChanged();

private:
    explicit Settings() = default;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SETTINGS_H

#ifndef REMOTECONTROL_SETTINGS_H
#define REMOTECONTROL_SETTINGS_H

#include <QObject>

namespace RemoteControl {

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int thumbnailSize READ thumbnailSize WRITE setThumbnailSize NOTIFY thumbnailSizeChanged)
    Q_PROPERTY(int categoryItemSize READ categoryItemSize WRITE setCategoryItemSize NOTIFY categoryItemSizeChanged)

    double m_thumbnailScale;

public:
    static Settings& instance();
    int thumbnailSize() const;
    void setThumbnailSize(int size);
    int categoryItemSize() const;

public slots:
    void setCategoryItemSize(int size);

signals:
    void thumbnailSizeChanged();
    void categoryItemSizeChanged();

private:
    explicit Settings() = default;
    int m_categoryItemSize;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SETTINGS_H

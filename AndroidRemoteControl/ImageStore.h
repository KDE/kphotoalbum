#ifndef REMOTECONTROL_IMAGESTORE_H
#define REMOTECONTROL_IMAGESTORE_H

#include <QObject>
#include <QImage>
#include <QMap>

namespace RemoteControl {

class ImageStore : public QObject
{
    Q_OBJECT
public:
    static ImageStore& instance();
    void updateImage(const QString& fileName, const QImage& image);
    QImage image(const QString& fileName) const;


signals:
    void imageUpdated(const QString& fileName);

private:
    explicit ImageStore() = default;

    QMap<QString,QImage> m_imageMap;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGESTORE_H

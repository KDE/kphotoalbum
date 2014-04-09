#ifndef REMOTECONTROL_IMAGESTORE_H
#define REMOTECONTROL_IMAGESTORE_H

#include <QObject>
#include <QImage>
#include <QMap>
#include "Types.h"

namespace RemoteControl {

class ImageStore : public QObject
{
    Q_OBJECT
public:
    static ImageStore& instance();
    void updateImage(const QString& fileName, const QImage& image);
    QImage image(const QString& fileName, const QSize& size, ViewType type) const;


signals:
    void imageUpdated(const QString& fileName, ViewType type);

private slots:
    void reset();

private:
    explicit ImageStore();
    void requestImage(const QString& fileName, const QSize& size, ViewType type) const;

    QMap<QPair<QString,ViewType>,QImage> m_imageMap;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGESTORE_H

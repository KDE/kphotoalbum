#ifndef REMOTECONTROL_IMAGESTORE_H
#define REMOTECONTROL_IMAGESTORE_H

#include <QObject>
#include <QImage>
#include <QMap>
#include "Types.h"
#include <QMutex>

namespace RemoteControl {
class RemoteImage;

class ImageStore : public QObject
{
    Q_OBJECT
public:
    static ImageStore& instance();
    void updateImage(ImageId imageId, const QImage& image, const QString& label, ViewType type);
    QImage image(RemoteImage* client, ImageId imageId, const QSize& size, ViewType type);
    QString label(int imageId) const;

private slots:
    void reset();
    void clientDeleted();

private:
    explicit ImageStore();
    void requestImage(RemoteImage* client, ImageId imageId, const QSize& size, ViewType type);

    using RequestType = QPair<ImageId,ViewType>;
    QMap<RequestType,QImage> m_imageMap;
    QMap<ImageId,QString> m_labelMap;
    QMap<RequestType,RemoteImage*> m_requestMap;
    QMap<RemoteImage*,RequestType> m_reverseRequestMap;
    QMutex m_mutex;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_IMAGESTORE_H

#pragma once

#include "../RemoteControl/RemoteCommand.h"
#include <QQuickImageProvider>

class ImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
    Q_PROPERTY(bool ready MEMBER m_ready NOTIFY imagesChanged)
    Q_PROPERTY(QImage home READ homeIcon NOTIFY imagesChanged)
    Q_PROPERTY(QImage kphotoalbum READ kphotoalbumIcon NOTIFY imagesChanged)
    Q_PROPERTY(QImage discovery READ discoverIcon NOTIFY imagesChanged)
    Q_PROPERTY(QImage info READ info NOTIFY imagesChanged)
    Q_PROPERTY(QImage slideShow READ slideShow NOTIFY imagesChanged)

public:
    ImageProvider();
    static ImageProvider &instance();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    void setImages(const RemoteControl::StaticImageResult &images);

    const QImage &homeIcon() const;
    const QImage &kphotoalbumIcon() const;
    const QImage &discoverIcon() const;
    const QImage &info() const;
    const QImage &slideShow() const;

signals:
    void imagesChanged();

private:
    bool m_ready = false;
    RemoteControl::StaticImageResult m_images;
};

#pragma once

#include <QQuickImageProvider>

class ImageProvider : public QQuickImageProvider
{
public:
    static ImageProvider &instance();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    QImage m_info;
    QImage m_slideShow;
    QImage m_search;

private:
    ImageProvider();
};

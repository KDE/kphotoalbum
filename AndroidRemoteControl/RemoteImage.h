#ifndef REMOTEIMAGE_H
#define REMOTEIMAGE_H

#include <QQuickPaintedItem>
#include <QImage>
#include "Types.h"

namespace RemoteControl {

class RemoteImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int imageId READ imageId WRITE setImageId NOTIFY imageIdChanged)
    Q_PROPERTY(RemoteControl::Types::ViewType type MEMBER m_type NOTIFY typeChanged)
    Q_PROPERTY(QString label MEMBER m_label NOTIFY labelChanged)


public:
    explicit RemoteImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    int imageId() const;
    QSize size() const;
    void setLabel(const QString& label);
    void setImage(const QImage& image);

public slots:
    void setImageId(int imageId);

protected:
    void componentComplete();

private slots:
    void requestImage();

signals:
    void imageIdChanged();
    void typeChanged();
    void labelChanged();

private:
    int m_imageId;
    ViewType m_type;
    QString m_label;
    QImage m_image;
};

}
#endif // REMOTEIMAGE_H

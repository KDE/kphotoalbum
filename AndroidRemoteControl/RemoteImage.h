#ifndef REMOTEIMAGE_H
#define REMOTEIMAGE_H

#include <QQuickPaintedItem>
#include <QImage>

namespace RemoteControl {

class RemoteImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY( bool connected READ isConnected NOTIFY connectionChanged )

public:
    explicit RemoteImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    bool isConnected() const;

signals:
    void connectionChanged();

private slots:
    void setImage(const QImage&);

private:
    QImage m_image;
};

}
#endif // REMOTEIMAGE_H

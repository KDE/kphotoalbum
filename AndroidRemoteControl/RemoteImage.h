#ifndef REMOTEIMAGE_H
#define REMOTEIMAGE_H

#include <QQuickPaintedItem>
#include <QImage>

namespace RemoteControl {

class RemoteImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexChanged)

public:
    explicit RemoteImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    int index() const;

public slots:
    void setIndex(int index);

signals:
    void indexChanged();

private slots:
    void updateImage(int index);

private:
    int m_index;
};

}
#endif // REMOTEIMAGE_H

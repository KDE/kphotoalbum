#ifndef MYIMAGE_H
#define MYIMAGE_H

#include <QQuickPaintedItem>
#include <QImage>

namespace RemoteControl
{

class MyImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)

public:
    explicit MyImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    QImage image() const;

public slots:
    void setImage(const QImage& image);

signals:
    void imageChanged();

private:
    QImage m_image;
};

}
#endif // MYIMAGE_H

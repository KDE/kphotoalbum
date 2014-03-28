#ifndef REMOTEIMAGE_H
#define REMOTEIMAGE_H

#include <QQuickPaintedItem>
#include <QImage>

namespace RemoteControl {

class RemoteImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)

public:
    explicit RemoteImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    QString fileName() const;

public slots:
    void setFileName(const QString& fileName);

signals:
    void fileNameChanged();

private slots:
    void updateImage(const QString& fileName);

private:
    QString m_fileName;
};

}
#endif // REMOTEIMAGE_H

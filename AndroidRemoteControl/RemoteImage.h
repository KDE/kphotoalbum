#ifndef REMOTEIMAGE_H
#define REMOTEIMAGE_H

#include <QQuickPaintedItem>
#include <QImage>
#include "Types.h"

namespace RemoteControl {

class RemoteImage : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    //Q_PROPERTY(ViewType type MEMBER m_type NOTIFY typeChanged)
    Q_PROPERTY(bool isThumbnail MEMBER m_isThumbnail NOTIFY isThumbnailChanged) // PENDING(blackie) remove!

    //Q_ENUMS(ViewType);
public:

    explicit RemoteImage(QQuickItem *parent = 0);
    void paint(QPainter *painter) override;
    QString fileName() const;
    QSize size() const;

public slots:
    void setFileName(const QString& fileName);

signals:
    void fileNameChanged();
    //void typeChanged();
    void isThumbnailChanged();

private slots:
    void updateImage(const QString& fileName, ViewType type);

private:
    QString m_fileName;
    //ViewType m_type = ViewType::Thumbnail;
    bool m_isThumbnail = true;
};

}
#endif // REMOTEIMAGE_H

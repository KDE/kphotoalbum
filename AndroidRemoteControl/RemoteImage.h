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
    Q_PROPERTY(int type MEMBER m_type NOTIFY typeChanged) // Should be ViewType

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
    void typeChanged();

private slots:
    void updateImage(const QString& fileName, ViewType type);

private:
    QString m_fileName;
    int m_type;
};

}
#endif // REMOTEIMAGE_H

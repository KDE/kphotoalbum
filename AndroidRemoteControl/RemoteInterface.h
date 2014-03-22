#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include "RemoteCommand.h"

#include <QObject>
#include <QMap>
#include <QImage>

class QTcpSocket;

namespace RemoteControl {

class RemoteConnection;

class RemoteInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool connected READ isConnected NOTIFY connectionChanged )
    Q_PROPERTY(int imageCount MEMBER m_imageCount NOTIFY imageCountChanged)

public:
    static RemoteInterface& instance();
    bool isConnected() const;
    QImage image(int index) const;

public slots:
    void previousSlide();
    void nextSlide();

signals:
    void connectionChanged();
    void imageUpdated(int index);
    void imageCountChanged();

private slots:
    void handleCommand(const RemoteCommand&);
    void updateImage(const ImageUpdateCommand&);
    void updateImageCount(const ImageCountUpdateCommand&);

private:
    RemoteInterface();

    RemoteConnection* m_connection = nullptr;
    int m_imageCount = 0;
    QMap<int,QImage> m_imageMap;
};

}

#endif // REMOTEINTERFACE_H

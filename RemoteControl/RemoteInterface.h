#ifndef REMOTEINTERFACE_H
#define REMOTEINTERFACE_H

#include <QObject>
#include <QHostAddress>


namespace RemoteControl
{
class Server;
class RemoteCommand;
class NextSlideCommand;
class PreviousSlideCommand;
class RemoteInterface : public QObject
{
    Q_OBJECT
public:
    void sendImage(const QImage& image);
    static RemoteInterface& instance();

private slots:
    void sendPage();
    void handleCommand(const RemoteCommand&);
    void sendInitialData();

private:
    explicit RemoteInterface(QObject *parent = 0);
    Server* m_connection;
};

}
#endif // REMOTEINTERFACE_H

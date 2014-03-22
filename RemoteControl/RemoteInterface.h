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
    void sendImage(int index, const QImage& image);
    static RemoteInterface& instance();
    void sendImageCount(int count);

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

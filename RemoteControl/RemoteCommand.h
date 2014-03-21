#ifndef REMOTECOMMAND_H
#define REMOTECOMMAND_H

#include <QString>
#include <QDataStream>
#include <QImage>

class RemoteCommand
{
public:
    RemoteCommand(const QString& id);
    virtual ~RemoteCommand() = default;
    virtual void encodeData(QDataStream&) const {};
    virtual void decodeData(QDataStream&) {};
    QString id() const;

    static RemoteCommand& command(const QString& id);

private:
    QString m_id;
};

class NextSlideCommand :public RemoteCommand {
public:
    NextSlideCommand();
    static QString id();
};

class PreviousSlideCommand :public RemoteCommand {
public:
    PreviousSlideCommand();
    static QString id();
};

class ImageUpdateCommand :public RemoteCommand {
public:
    ImageUpdateCommand();
    static QString id();
    void encodeData(QDataStream& buffer) const override;
    void decodeData(QDataStream&) override;
    QImage image;
};



#endif // REMOTECOMMAND_H

/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "../RemoteControl/Types.h"
#include "../RemoteControl/VideoSharedTypes.h"
#include <QObject>

class QTcpSocket;
class QTcpServer;

namespace RemoteControl
{
/**
 * @brief The VideoClient class fetches videos over TCP from the desktop application
 *
 * Sorry for the confusion, but for technical reasons this actually run the TCP Server.
 */
class VideoClient : public QObject
{
    Q_OBJECT
public:
    explicit VideoClient(QObject *parent = nullptr);

private:
    void setupTCPServer();
    void acceptConnection();
    void disconnect();
    void dataReceived();

    enum ReadingState { WaitingForLength,
                        WaitingForData };
    ReadingState m_state = WaitingForLength;
    qint32 m_length;

    RemoteControl::ImageId m_imageId;
    int m_fileSize;
    int m_currentDataReceived;
    QTcpServer *m_server = nullptr;
    QTcpSocket *m_socket = nullptr;
};

}

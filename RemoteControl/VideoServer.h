/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "Types.h"
#include <QHostAddress>
#include <QObject>
#include <kpabase/FileName.h>

class QTcpSocket;

namespace RemoteControl
{
class VideoServer : public QObject
{
    Q_OBJECT
public:
    explicit VideoServer(QObject *parent = nullptr);
    void connectToTCPServer(const QHostAddress &address);
    void sendVideo(const DB::FileName &fileName, ImageId imageId);

private:
    void gotConnected();
    void dataReceived();
    void lostConnection();

    QTcpSocket *m_socket = nullptr;
};

} // namespace RemoteControl

// SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef REMOTECONTROL_CONNECTIONINDICATOR_H
#define REMOTECONTROL_CONNECTIONINDICATOR_H

#include <QLabel>

class QTimer;

namespace RemoteControl
{

class ConnectionIndicator : public QLabel
{
    Q_OBJECT
public:
    explicit ConnectionIndicator(QWidget *parent = 0);

    void mouseReleaseEvent(QMouseEvent *ev) override;
    void contextMenuEvent(QContextMenuEvent *ev) override;
private Q_SLOTS:
    void on();
    void off();
    void wait();
    void waitingAnimation();

private:
    enum State { Off,
                 Connecting,
                 On };
    State m_state;
    QTimer *m_timer;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_CONNECTIONINDICATOR_H

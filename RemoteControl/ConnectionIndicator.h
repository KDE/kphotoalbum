/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

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
private slots:
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

/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BACKGROUNDTASKS_STATUSINDICATOR_H
#define BACKGROUNDTASKS_STATUSINDICATOR_H

#include <KLed>

class QTimer;
class QHelpEvent;

namespace BackgroundTaskManager
{
class JobViewer;

class StatusIndicator : public KLed
{
    Q_OBJECT

public:
    explicit StatusIndicator(QWidget *parent);
    bool event(QEvent *event) override;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void flicker();
    void maybeStartFlicker();

private:
    QColor currentColor() const;
    void showToolTip(QHelpEvent *event);
    QTimer *m_timer;
    JobViewer *m_jobViewer;
};

} // namespace BackgroundTaskManager

#endif // BACKGROUNDTASKS_STATUSINDICATOR_H
// vi:expandtab:tabstop=4 shiftwidth=4:

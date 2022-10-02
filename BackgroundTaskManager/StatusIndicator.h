// SPDX-FileCopyrightText: 2012-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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

private Q_SLOTS:
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

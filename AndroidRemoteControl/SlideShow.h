/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
class QTimer;

class SlideShow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)
    Q_PROPERTY(int interval READ interval WRITE setInterval NOTIFY intervalChanged)
    Q_PROPERTY(bool videoRunning READ videoRunning WRITE setVideoRunning NOTIFY videoRunningChanged)

public:
    explicit SlideShow(QObject *parent = nullptr);

    bool running() const;
    void setRunning(bool newRunning);
    Q_INVOKABLE void setOverride(bool newRunning);

    int interval() const;
    void setInterval(int newInterval);

    bool videoRunning() const;
    void setVideoRunning(bool newVideoRunning);

signals:
    void runningChanged();
    void requestNext();
    void intervalChanged();

    void videoRunningChanged();

private:
    bool m_running = false;
    bool m_override = false;
    QTimer *m_timer;
    bool m_videoRunning = false;
};

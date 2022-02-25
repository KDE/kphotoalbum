/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SlideShow.h"
#include <QDebug>
#include <QTimer>
#include <chrono>

using namespace std::chrono_literals;

SlideShow::SlideShow(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(4s);
    connect(m_timer, &QTimer::timeout, this, &SlideShow::requestNext);
}

bool SlideShow::running() const
{
    return m_running;
}

void SlideShow::setRunning(bool newRunning)
{
    if (m_running == newRunning)
        return;
    m_running = newRunning;
    emit runningChanged();

    if (newRunning && !m_videoRunning)
        m_timer->start();
    else
        m_timer->stop();
}

void SlideShow::setOverride(bool newRunning)
{
    if (newRunning)
        setRunning(m_override);
    else {
        m_override = m_running;
        setRunning(false);
    }
}

int SlideShow::interval() const
{
    return m_timer->interval() / 1000;
}

void SlideShow::setInterval(int interval)
{
    if (m_timer->interval() / 1000 == interval)
        return;
    m_timer->setInterval(interval * 1000);
    emit intervalChanged();
}

bool SlideShow::videoRunning() const
{
    return m_videoRunning;
}

void SlideShow::setVideoRunning(bool newVideoRunning)
{
    if (m_videoRunning == newVideoRunning)
        return;
    m_videoRunning = newVideoRunning;
    emit videoRunningChanged();

    if (m_videoRunning) {
        m_timer->stop();
    } else if (m_running) {
        m_timer->start();
        // This need to be delayed, otherwise we get a binding loop on the QML site.
        QMetaObject::invokeMethod(this, "requestNext", Qt::QueuedConnection);
    }
}

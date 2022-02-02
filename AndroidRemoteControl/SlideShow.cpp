/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SlideShow.h"
#include <QTimer>

SlideShow::SlideShow(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(4000);
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

    if (newRunning)
        m_timer->start();
    else
        m_timer->stop();
}

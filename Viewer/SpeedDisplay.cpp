/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include "SpeedDisplay.h"

#include <QLabel>
#include <QLayout>
#include <QTimeLine>
#include <QTimer>

#include <KLocalizedString>

Viewer::SpeedDisplay::SpeedDisplay(QWidget *parent)
    : QLabel(parent)
{
    m_timeLine = new QTimeLine(1000, this);
    connect(m_timeLine, SIGNAL(frameChanged(int)), this, SLOT(setAlphaChannel(int)));
    m_timeLine->setFrameRange(0, 170);
    m_timeLine->setDirection(QTimeLine::Backward);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, m_timeLine, &QTimeLine::start);

    setAutoFillBackground(true);
}

void Viewer::SpeedDisplay::display(int i)
{
    // FIXME(jzarl): if the user sets a different shortcut, this is inaccurate
    // -> dynamically update this text
    setText(i18nc("OSD for slideshow, num of seconds per image", "<p><center><font size=\"+4\">%1&nbsp;s</font></center></p>", i / 1000.0));
    go();
}

void Viewer::SpeedDisplay::start()
{
    // FIXME(jzarl): if the user sets a different shortcut, this is inaccurate
    // -> dynamically update this text
    setText(i18nc("OSD for slideshow", "<p><center><font size=\"+4\">Starting Slideshow<br/>Ctrl++ makes the slideshow faster<br/>Ctrl + - makes the slideshow slower</font></center></p>"));
    go();
}

void Viewer::SpeedDisplay::go()
{
    resize(sizeHint());
    QWidget *p = static_cast<QWidget *>(parent());
    move((p->width() - width()) / 2, (p->height() - height()) / 2);

    setAlphaChannel(170, 255);
    m_timer->start(1000);
    m_timeLine->stop();

    show();
    raise();
}

void Viewer::SpeedDisplay::end()
{
    setText(i18nc("OSD for slideshow", "<p><center><font size=\"+4\">Ending Slideshow</font></center></p>"));
    go();
}

void Viewer::SpeedDisplay::setAlphaChannel(int background, int label)
{
    QPalette p = palette();
    p.setColor(QPalette::Background, QColor(0, 0, 0, background)); // r,g,b,A
    p.setColor(QPalette::WindowText, QColor(255, 255, 255, label));
    setPalette(p);
}

void Viewer::SpeedDisplay::setAlphaChannel(int alpha)
{
    setAlphaChannel(alpha, alpha);
    if (alpha == 0)
        hide();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

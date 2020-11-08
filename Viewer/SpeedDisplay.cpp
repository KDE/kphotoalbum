/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpeedDisplay.h"

#include <KLocalizedString>
#include <QLabel>
#include <QLayout>
#include <QTimeLine>
#include <QTimer>

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

void Viewer::SpeedDisplay::setAlphaChannel(int backgroundAlpha, int labelAlpha)
{
    QPalette p = palette();
    QColor bgColor = p.window().color();
    bgColor.setAlpha(backgroundAlpha);
    p.setColor(QPalette::Background, bgColor);
    QColor fgColor = p.windowText().color();
    fgColor.setAlpha(labelAlpha);
    p.setColor(QPalette::WindowText, fgColor);
    setPalette(p);
    // re-enable palette propagation:
    setAttribute(Qt::WA_SetPalette);
}

void Viewer::SpeedDisplay::setAlphaChannel(int alpha)
{
    setAlphaChannel(alpha, alpha);
    if (alpha == 0)
        hide();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

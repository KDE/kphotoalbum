// SPDX-FileCopyrightText: 2003-2023 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TransientDisplay.h"

#include <KLocalizedString>
#include <QLabel>
#include <QLayout>
#include <QTimeLine>
#include <QTimer>

Viewer::TransientDisplay::TransientDisplay(QWidget *parent)
    : QLabel(parent)
{
    m_timeLine = new QTimeLine(1000, this);
    connect(m_timeLine, &QTimeLine::frameChanged, this, qOverload<int>(&TransientDisplay::setAlphaChannel));
    m_timeLine->setFrameRange(0, 170);
    m_timeLine->setDirection(QTimeLine::Backward);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, [this] {
        if (m_nextFadeAction == FadeOut)
            m_timeLine->start();
        else
            hide();
    });

    setAutoFillBackground(true);
}

void Viewer::TransientDisplay::display(const QString &text, std::chrono::milliseconds duration, FadeAction action)
{
    setText(QLatin1String("<p><center><font size=\"+4\">%1</font></center></p>").arg(text));
    m_nextFadeAction = action;
    go(duration);
}

void Viewer::TransientDisplay::go(std::chrono::milliseconds duration)
{
    resize(sizeHint());
    QWidget *p = static_cast<QWidget *>(parent());
    move((p->width() - width()) / 2, (p->height() - height()) / 2);

    setAlphaChannel(170, 255);
    m_timer->start(duration);
    m_timeLine->stop();

    show();
    raise();
}

void Viewer::TransientDisplay::setAlphaChannel(int backgroundAlpha, int labelAlpha)
{
    QPalette p = palette();
    QColor bgColor = p.window().color();
    bgColor.setAlpha(backgroundAlpha);
    p.setColor(QPalette::Window, bgColor);
    QColor fgColor = p.windowText().color();
    fgColor.setAlpha(labelAlpha);
    p.setColor(QPalette::WindowText, fgColor);
    setPalette(p);
    // re-enable palette propagation:
    setAttribute(Qt::WA_SetPalette);
}

void Viewer::TransientDisplay::setAlphaChannel(int alpha)
{
    setAlphaChannel(alpha, alpha);
    if (alpha == 0)
        hide();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_TransientDisplay.cpp"

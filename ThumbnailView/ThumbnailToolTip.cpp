// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2005-2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007 Thiago Macieira <thiago@kde.org>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2008-2010 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2010 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2018 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ThumbnailToolTip.h"

#include "ThumbnailWidget.h"

#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <kpabase/FileUtil.h>
#include <kpabase/SettingsData.h>

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QScreen>

/**
   \class ThumbnailToolTip
   This class takes care of showing tooltips for the individual items in the thumbnail view.
   I tried implementing this with QToolTip::maybeTip() on the iconview, but it had the
   disadvantages that either the tooltip would not follow the
   mouse( and would therefore stand on top of the image), or it flickered.
*/

ThumbnailView::ThumbnailToolTip::ThumbnailToolTip(ThumbnailWidget *view)
    : Utilities::ToolTip(view, Qt::FramelessWindowHint | Qt::Window | Qt::X11BypassWindowManagerHint | Qt::Tool)
    , m_view(view)
    , m_widthInverse(false)
    , m_heightInverse(false)
{
}

bool ThumbnailView::ThumbnailToolTip::eventFilter(QObject *o, QEvent *event)
{
    if (o == m_view->viewport() && event->type() == QEvent::Leave)
        hide();

    else if (event->type() == QEvent::MouseMove || event->type() == QEvent::Wheel) {
        // We need this to be done through a timer, so the thumbnail view gets the wheel even first,
        // otherwise the fileName reported by mediaIdUnderCursor is wrong.
        QTimer::singleShot(0, this, SLOT(requestToolTip()));
    }

    return false;
}

void ThumbnailView::ThumbnailToolTip::requestToolTip()
{
    const DB::FileName fileName = m_view->mediaIdUnderCursor();
    ToolTip::requestToolTip(fileName);
}

void ThumbnailView::ThumbnailToolTip::setActive(bool b)
{
    if (b) {
        requestToolTip();
        m_view->viewport()->installEventFilter(this);
    } else {
        m_view->viewport()->removeEventFilter(this);
        hide();
    }
}

void ThumbnailView::ThumbnailToolTip::placeWindow()
{
    // First try to set the position.
    QPoint pos = QCursor::pos() + QPoint(20, 20);
    if (m_widthInverse)
        pos.setX(pos.x() - 30 - width());
    if (m_heightInverse)
        pos.setY(pos.y() - 30 - height());

    QScreen *screen = qApp->screenAt(QCursor::pos());
    if (!screen)
        return;
    QRect geom = screen->geometry();

    // Now test whether the window moved outside the screen
    if (m_widthInverse) {
        if (pos.x() < geom.x()) {
            pos.setX(QCursor::pos().x() + 20);
            m_widthInverse = false;
        }
    } else {
        if (pos.x() + width() > geom.right()) {
            pos.setX(QCursor::pos().x() - width());
            m_widthInverse = true;
        }
    }

    if (m_heightInverse) {
        if (pos.y() < geom.y()) {
            pos.setY(QCursor::pos().y() + 10);
            m_heightInverse = false;
        }
    } else {
        if (pos.y() + height() > geom.bottom()) {
            pos.setY(QCursor::pos().y() - 10 - height());
            m_heightInverse = true;
        }
    }

    move(pos);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ThumbnailToolTip.cpp"

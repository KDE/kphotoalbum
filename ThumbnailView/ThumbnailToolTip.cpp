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

#include "ThumbnailToolTip.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QCursor>
#include <QScreen>

#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "Settings/SettingsData.h"
#include "ThumbnailWidget.h"
#include "Utilities/Util.h"

/**
   \class ThumbnailToolTip
   This class takes care of showing tooltips for the individual items in the thumbnail view.
   I tried implementing this with QToolTip::maybeTip() on the iconview, but it had the
   disadvantages that either the tooltip would not follow the
   mouse( and would therefore stand on top of the image), or it flickered.
*/

ThumbnailView::ThumbnailToolTip::ThumbnailToolTip( ThumbnailWidget* view )
    : Utilities::ToolTip(view, Qt::FramelessWindowHint | Qt::Window
                         | Qt::X11BypassWindowManagerHint
                         | Qt::Tool ), m_view( view ),
      m_widthInverse( false ), m_heightInverse( false )
{
}

bool ThumbnailView::ThumbnailToolTip::eventFilter( QObject* o , QEvent* event )
{
    if ( o == m_view->viewport() && event->type() == QEvent::Leave )
        hide();

    else if ( event->type() == QEvent::MouseMove || event->type() == QEvent::Wheel) {
        // We need this to be done through a timer, so the thumbnail view gets the wheel even first,
        // otherwise the fileName reported by mediaIdUnderCursor is wrong.
        QTimer::singleShot(0,this, SLOT(requestToolTip()));
    }

    return false;
}

void ThumbnailView::ThumbnailToolTip::requestToolTip()
{
    const DB::FileName fileName = m_view->mediaIdUnderCursor();
    ToolTip::requestToolTip(fileName);
}


void ThumbnailView::ThumbnailToolTip::setActive( bool b )
{
    if ( b ) {
        requestToolTip();
        m_view->viewport()->installEventFilter( this );
    }
    else {
        m_view->viewport()->removeEventFilter( this );
        hide();
    }
}

void ThumbnailView::ThumbnailToolTip::placeWindow()
{
    // First try to set the position.
    QPoint pos = QCursor::pos() + QPoint( 20, 20 );
    if ( m_widthInverse )
        pos.setX( pos.x() - 30 - width() );
    if ( m_heightInverse )
        pos.setY( pos.y() - 30 - height() );

    // TODO: remove this version check once we don't care about Ubuntu 18.04 LTS anymore
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QScreen *screen = qApp->screenAt( QCursor::pos());
    if (!screen)
        return;
    QRect geom = screen->geometry();
#else
    QRect geom = qApp->desktop()->screenGeometry( QCursor::pos() );
#endif

    // Now test whether the window moved outside the screen
    if ( m_widthInverse ) {
        if ( pos.x() <  geom.x() ) {
            pos.setX( QCursor::pos().x() + 20 );
            m_widthInverse = false;
        }
    }
    else {
        if ( pos.x() + width() > geom.right() ) {
            pos.setX( QCursor::pos().x() - width() );
            m_widthInverse = true;
        }
    }

    if ( m_heightInverse ) {
        if ( pos.y() < geom.y()  ) {
            pos.setY( QCursor::pos().y() + 10 );
            m_heightInverse = false;
        }
    }
    else {
        if ( pos.y() + height() > geom.bottom() ) {
            pos.setY( QCursor::pos().y() - 10 - height() );
            m_heightInverse = true;
        }
    }

    move( pos );
}

// vi:expandtab:tabstop=4 shiftwidth=4:

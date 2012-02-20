/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#include "Viewer/ViewHandler.h"
#include <QRubberBand>
#include <qpainter.h>
#include <qapplication.h>
#include <qcursor.h>
#include <QMouseEvent>

/**
 * \class Viewer::ViewHandler
 * \brief Mouse handler used during zooming and panning actions
 */

Viewer::ViewHandler::ViewHandler( Viewer::ImageDisplay* display )
    :QObject( display ), _scale( false ), _pan( false ), _rubberBand( new QRubberBand( QRubberBand::Rectangle, display ) ), _display(display)
{

}

bool Viewer::ViewHandler::mousePressEvent( QMouseEvent*e,  const QPoint& unTranslatedPos, double /*scaleFactor*/ )
{
    _pan = false;
    _scale = false;

    if ( (e->button() & Qt::LeftButton ) ) {
        if ( (e->modifiers() & Qt::ControlModifier ) ) {
            _pan = true;
        } else {
            _scale = true;
        }
    }
    else if ( e->button() & Qt::MidButton ) {
        _pan = true;
    }

    if (_pan) {
         // panning
        _last = unTranslatedPos;
        qApp->setOverrideCursor( Qt::SizeAllCursor );
        _errorX = 0;
        _errorY = 0;
        return true;
    } else if (_scale) {
        // scaling
        _start = e->pos();
        _untranslatedStart = unTranslatedPos;
        qApp->setOverrideCursor( Qt::CrossCursor );
        return true;
    } else {
        return true;
    }
}

bool Viewer::ViewHandler::mouseMoveEvent( QMouseEvent*,
                                          const QPoint& unTranslatedPos, double scaleFactor )
{
    if ( _scale ) {
        _rubberBand->setGeometry( QRect( _untranslatedStart, unTranslatedPos ) );
        _rubberBand->show();
        return true;
    }
    else if ( _pan ) {
        // This code need to be taking the error into account, consider this situation:
        // The user moves the mouse very slowly, only 1 pixel at a time, scale factor is 3
        // Then translated delta would be 1/3 which every time would be
        // rounded down to 0, and the panning would never move any pixels.
        double deltaX = _errorX + (_last.x() - unTranslatedPos.x())/scaleFactor;
        double deltaY = _errorY + (_last.y() - unTranslatedPos.y())/scaleFactor;
        QPoint deltaPoint = QPoint( (int) deltaX, (int) deltaY );
        _errorX = deltaX - ((double) ((int) deltaX ) );
        _errorY = deltaY - ((double) ((int) deltaY) );

        _display->pan( deltaPoint );
        _last = unTranslatedPos;
        return true;
    }
    else
        return false;
}

bool Viewer::ViewHandler::mouseReleaseEvent( QMouseEvent* e,  const QPoint& /*unTranslatedPos*/, double /*scaleFactor*/ )
{
    if ( _scale ) {
        qApp->restoreOverrideCursor();
        _rubberBand->hide();
        _scale = false;
        if ( (e->pos()-_start).manhattanLength() > 1 ) {
            _display->zoom( _start, e->pos() );
            return true;
        } else
            return false;
    }
    else if ( _pan ) {
        qApp->restoreOverrideCursor();
        _pan = false;
        return true;
    }
    else
      return false;
}

void Viewer::ViewHandler::hideEvent()
{
  // In case the escape key is pressed while viewing or scaling, then we need to restore the override cursor
  // (As in that case we will not see a key release event)
  if ( _pan || _scale) {
    qApp->restoreOverrideCursor();
    _pan = false;
    _scale = false;
  }
}

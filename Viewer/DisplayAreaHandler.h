/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DISPLAYAREAHANDLER_H
#define DISPLAYAREAHANDLER_H
class QMouseEvent;
#include <qobject.h>
#include "Viewer/DisplayArea.h"

namespace Viewer
{

/**
 * \brief Mouse event handler for Viewer.
 *
 * Mouse actions in the viewer depends on the current actions, which is
 * either drawing on an image or not. In the "not" case the mouse actions
 * means zoom or scroll.
 *
 * To separate the code for the two separate cases, this class is an
 * interface each case needs to implement for mouse handling.
 */
class DisplayAreaHandler :public QObject
{
public:
    DisplayAreaHandler( ImageDisplay* display ) : QObject( display, "display handler" ), _display( display ) {}
    virtual bool mousePressEvent ( QMouseEvent* e, const QPoint& /*unTranslatedPos*/, double scaleFactor ) = 0;
    virtual bool mouseReleaseEvent ( QMouseEvent* e, const QPoint& /*unTranslatedPos*/, double scaleFactor ) = 0;
    virtual bool mouseMoveEvent ( QMouseEvent* e, const QPoint& /*unTranslatedPos*/, double scaleFactor ) = 0;

protected:
    ImageDisplay* _display;
};

}

#endif /* DISPLAYAREAHANDLER_H */


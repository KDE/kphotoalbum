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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef VIEWHANDLER_H
#define VIEWHANDLER_H
#include "displayareahandler.h"
#include <qpoint.h>

class ViewHandler :public DisplayAreaHandler {

public:
    ViewHandler( DisplayArea* display );
    virtual bool mousePressEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
    virtual bool mouseReleaseEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
    virtual bool mouseMoveEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
private:
    bool _scale, _pan;
    QPoint _start, _last;
    double _errorX, _errorY;
};


#endif /* VIEWHANDLER_H */


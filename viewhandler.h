/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef VIEWHANDLER_H
#define VIEWHANDLER_H
#include "displayareahandler.h"
#include <qpoint.h>

class ViewHandler :public DisplayAreaHandler {

public:
    ViewHandler( DisplayArea* display );
    virtual bool mousePressEvent ( QMouseEvent* e );
    virtual bool mouseReleaseEvent ( QMouseEvent* e );
    virtual bool mouseMoveEvent ( QMouseEvent* e );
private:
    bool _scale, _pan;
    QPoint _start, _last;
};


#endif /* VIEWHANDLER_H */


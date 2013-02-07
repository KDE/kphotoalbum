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

#ifndef VIEWHANDLER_H
#define VIEWHANDLER_H
#include <qpoint.h>
#include <QMouseEvent>
#include "ImageDisplay.h"

class ImageDisplay;
class QRubberBand;

namespace Viewer
{

class ViewHandler :public QObject {
Q_OBJECT
public:
    ViewHandler( ImageDisplay* display );
    bool mousePressEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
    bool mouseReleaseEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
    bool mouseMoveEvent ( QMouseEvent* e, const QPoint& unTranslatedPos, double scaleFactor );
    void hideEvent();

private:
    bool _scale, _pan;
    QPoint _start, _untranslatedStart, _last;
    double _errorX, _errorY;
    QRubberBand* _rubberBand;
    ImageDisplay* _display;
};

}

#endif /* VIEWHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

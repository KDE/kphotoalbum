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

#ifndef VIEWER_DISPLAY_H
#define VIEWER_DISPLAY_H

#include <qwidget.h>
#include <DB/ImageInfoPtr.h>

namespace Viewer
{
class DrawHandler;

class Display :public QWidget
{
    Q_OBJECT

public:
    Display( QWidget* parent, const char* name = 0 );
    virtual bool setImage( DB::ImageInfoPtr info, bool forward ) = 0;

    virtual bool offersDrawOnImage() const { return false; }
    virtual DrawHandler* drawHandler() { return 0; }
    virtual void startDrawing() {}
    virtual void stopDrawing() {}

public slots:
    virtual void zoomIn() = 0;
    virtual void zoomOut() = 0;
    virtual void zoomFull() = 0;
    virtual void zoomPixelForPixel() = 0;

protected:
    DB::ImageInfoPtr _info;
};

}

#endif /* VIEWER_DISPLAY_H */


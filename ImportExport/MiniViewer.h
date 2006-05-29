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

#ifndef MINIVIEWER_H
#define MINIVIEWER_H

#include <qdialog.h>
#include <qimage.h>
#include "DB/ImageInfoPtr.h"
class QCloseEvent;
class QLabel;

namespace DB
{
    class ImageInfo;
}

namespace ImportExport
{

class MiniViewer :public QDialog
{
    Q_OBJECT

public:
    static void show( QImage img, DB::ImageInfoPtr info );
    virtual void closeEvent( QCloseEvent* event );

protected slots:
    void slotClose();

private:
    MiniViewer();
    static MiniViewer* _instance;
    QLabel* _pixmap;
};

}

#endif /* MINIVIEWER_H */


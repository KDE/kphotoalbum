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

#ifndef THUMBNAILBUILDER_H
#define THUMBNAILBUILDER_H

#include "imageclient.h"
#include <qprogressdialog.h>
#include <qimage.h>
#include "imageinfolist.h"


namespace ThumbnailView
{

class ThumbnailBuilder :public QProgressDialog, public ImageClient {
    Q_OBJECT

public:
    ThumbnailBuilder( QWidget* parent, const char* name = 0 );
    void generateNext();
    virtual void pixmapLoaded( const QString& fileName, const QSize& size, const QSize& fullSize, int angle, const QImage&, bool loadedOK );

private:
    QStringList _images;
    uint _index;
    QMap<QString, ImageInfoPtr> _infoMap;
};

}

#endif /* THUMBNAILBUILDER_H */


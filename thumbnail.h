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

#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <qlabel.h>
#include <qiconview.h>
#include "imageclient.h"
class ThumbNailView;
class ImageInfo;

class ThumbNail :public QIconViewItem, public ImageClient {
public:
    friend class ThumbNailView;
    ThumbNail( ImageInfo* imageInfo,  ThumbNailView* parent );
    ThumbNail( ImageInfo* imageInfo,  ThumbNail* after, ThumbNailView* parent );

    virtual void pixmapLoaded( const QString&, int, int, int, const QImage& );
    QString fileName() const;
    ImageInfo* imageInfo();
    virtual bool acceptDrop ( const QMimeSource * mime ) const;

protected:
    void init();
    virtual void dragLeft();
    virtual void dragMove();
    virtual void dropped ( QDropEvent * e, const QValueList<QIconDragItem> & lst );
    bool atRightSizeOfItem();


private:
    ImageInfo* _imageInfo;
    QPixmap _pixmap;
    ThumbNailView* _parent;
};

#endif /* THUMBNAIL_H */


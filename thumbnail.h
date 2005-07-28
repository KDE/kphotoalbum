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

#ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <qlabel.h>
#include <qiconview.h>
#include "imageclient.h"
#include "imageinfoptr.h"
class QPixmapCache;
class ThumbNailView;

class ThumbNail :public QIconViewItem, public ImageClient {
public:
    friend class ThumbNailView;
    ThumbNail( const QString& fileName,  ThumbNailView* parent );
    ThumbNail( const QString& fileName,  ThumbNail* after, ThumbNailView* parent );

    virtual void pixmapLoaded( const QString&, const QSize& size, const QSize& fullSize, int, const QImage&, bool loadedOK );
    QString fileName() const;
    ImageInfoPtr imageInfo() const;
    virtual bool acceptDrop ( const QMimeSource * mime ) const;

protected:
    void init();
    virtual void dragLeft();
    virtual void dragMove();
    virtual void dropped ( QDropEvent * e, const QValueList<QIconDragItem> & lst );
    bool atRightSizeOfItem() const;
    virtual void calcRect( const QString& text = QString::null );
    virtual void paintItem ( QPainter * p, const QColorGroup & cg );
    virtual QPixmap* pixmap() const;
    void repaintItem();

    static QPixmapCache& pixmapCache();
    static QPixmap* emptyPixmap();
    static QPixmap& highlightPixmap();

private:
    QString _fileName;
    ThumbNailView* _parent;
    bool _highlightItem;
    mutable ImageInfoPtr _info; // Only to be used by method imageInfo()
};

#endif /* THUMBNAIL_H */


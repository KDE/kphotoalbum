/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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

#ifndef THUMBNAILVIEW_H
#define THUMBNAILVIEW_H
#include <kiconview.h>
#include "imageinfo.h"
class ImageManager;
class IconViewToolTip;

class ThumbNailView :public KIconView {
    Q_OBJECT
    friend class ThumbNail;

public:
    ThumbNailView( QWidget* parent,  const char* name = 0 );

public slots:
    void reload();
    void slotSelectAll();
    void slotCut();
    void slotPaste();
    void showToolTipsOnImages();

signals:
    void changed();

protected slots:
    void showImage( QIconViewItem* );
    virtual void startDrag();

protected:
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDropEvent( QDropEvent* e );
    void setHighlighted( ThumbNail* item );
    QPtrList<ThumbNail> selected() const;
    void reorder( ImageInfo* item, const ImageInfoList& list, bool after );
    QDragObject* dragObject();

private:
    ThumbNail* _currentHighlighted;
    IconViewToolTip* _iconViewToolTip;
};

#endif /* THUMBNAILVIEW_H */


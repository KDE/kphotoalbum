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

#ifndef THUMBNAILVIEW_H
#define THUMBNAILVIEW_H
#include <kiconview.h>
class QDateTime;
class ImageInfo;
class ImageInfoList;
class ImageDateRange;
class ImageManager;
class IconViewToolTip;
class ThumbNail;

class ThumbNailView :public KIconView {
    Q_OBJECT
    friend class ThumbNail;

public:
    ThumbNailView( QWidget* parent,  const char* name = 0 );
    static ThumbNailView* theThumbnailView();
    void makeCurrent( ImageInfo* info );

public slots:
    void reload();
    void slotSelectAll();
    void slotCut();
    void slotPaste();
    void showToolTipsOnImages( bool );
    void gotoDate( const ImageDateRange&, bool includeRanges );

signals:
    void changed();
    void fileNameChanged( const QString& );
    void currentDateChanged( const QDateTime& );

protected slots:
    void showImage( QIconViewItem* );
    virtual void startDrag();
    void slotOnItem( QIconViewItem* );
    void slotOnViewPort();
    void setupGrid();
    void emitDateChange();

protected:
    virtual void contentsDragMoveEvent( QDragMoveEvent *e );
    virtual void contentsDropEvent( QDropEvent* e );
    void setHighlighted( ThumbNail* item );
    void setDragLeft(  ThumbNail* item );
    QPtrList<ThumbNail> selected() const;
    void reorder( ImageInfo* item, const ImageInfoList& list, bool after );
    QDragObject* dragObject();
    virtual void drawBackground ( QPainter * p, const QRect & r );
    virtual void showEvent( QShowEvent* );

private:
    static ThumbNailView* _instance;
    ThumbNail* _currentHighlighted;
    IconViewToolTip* _iconViewToolTip;
    bool _blockMoveSignals;
};

#endif /* THUMBNAILVIEW_H */


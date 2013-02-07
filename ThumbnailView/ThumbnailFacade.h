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
#ifndef THUMBNAILFACADE_H
#define THUMBNAILFACADE_H
#include "ThumbnailFactory.h"
#include "ThumbnailWidget.h"
#include <DB/FileNameList.h>

namespace ThumbnailView
{
class ThumbnailModel;
class CellGeometry;
class ThumbnailPainter;
class ThumbnailToolTip;

class ThumbnailFacade :public QObject, public ThumbnailFactory
{
    Q_OBJECT
public:
    static ThumbnailFacade* instance();
    ThumbnailFacade();
    QWidget* gui();
    void setCurrentItem( const DB::FileName& fileName );
    void reload( SelectionUpdateMethod method );
    DB::FileNameList selection( ThumbnailView::SelectionMode mode = ExpandCollapsedStacks ) const;
    DB::FileNameList imageList(Order) const;
    DB::FileName mediaIdUnderCursor() const;
    DB::FileName currentItem() const;
    void setImageList(const DB::FileNameList& list);
    void setSortDirection( SortDirection );

public slots:
    void gotoDate( const DB::ImageDate& date, bool includeRanges );
    void selectAll();
    void showToolTipsOnImages( bool b );
    void toggleStackExpansion(const DB::FileName& id);
    void collapseAllStacks();
    void expandAllStacks();
    void updateDisplayModel();
    void changeSingleSelection(const DB::FileName& fileName);
    void slotRecreateThumbnail();

signals:
    void showImage( const DB::FileName& id );
    void showSelection();
    void fileIdUnderCursorChanged( const DB::FileName& id );
    void currentDateChanged( const QDateTime& );
    void selectionChanged(int numberOfItemsSelected );
    void collapseAllStacksEnabled(bool enabled);
    void expandAllStacksEnabled(bool enabled);

private:
    OVERRIDE ThumbnailModel* model();
    OVERRIDE CellGeometry* cellGeometry();
    OVERRIDE ThumbnailWidget* widget();

private:
    static ThumbnailFacade* _instance;
    CellGeometry* _cellGeometry;
    ThumbnailModel* _model;
    ThumbnailWidget* _widget;
    ThumbnailPainter* _painter;
    ThumbnailToolTip* _toolTip;
};
}

#endif /* THUMBNAILFACADE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

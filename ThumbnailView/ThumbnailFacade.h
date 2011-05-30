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
    void setCurrentItem( const DB::Id& id );
    void reload( SelectionUpdateMethod method );
    DB::IdList selection() const;
    DB::IdList imageList(Order) const;
    DB::Id mediaIdUnderCursor() const;
    DB::Id currentItem() const;
    void setImageList(const DB::IdList& list);
    void setSortDirection( SortDirection );

public slots:
    void gotoDate( const DB::ImageDate& date, bool includeRanges );
    void selectAll();
    void showToolTipsOnImages( bool b );
    void toggleStackExpansion(const DB::Id& id);
    void collapseAllStacks();
    void expandAllStacks();
    void updateDisplayModel();
    void changeSingleSelection(const DB::Id& id);
    void slotRecreateThumbnail();

signals:
    void showImage( const DB::Id& id );
    void showSelection();
    void fileIdUnderCursorChanged( const DB::Id& id );
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


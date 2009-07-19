/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
#include "ThumbnailFacade.h"
#include "Cell.h"
#include "ThumbnailCache.h"
#include "Settings/SettingsData.h"
#include "ImageManager/Manager.h"
#include "ThumbnailToolTip.h"
#include "ThumbnailPainter.h"
#include "ThumbnailModel.h"
#include "CellGeometry.h"
#include "ThumbnailWidget.h"

ThumbnailView::ThumbnailFacade* ThumbnailView::ThumbnailFacade::_instance = 0;
ThumbnailView::ThumbnailFacade::ThumbnailFacade()
    :_cellGeometry( new CellGeometry(this) ),
     _model( 0 ), _thumbnailCache( 0 ),_widget( 0 ), _painter( 0 ), _toolTip( 0 )
{
    // To avoid one of the components references one of the other before it has been initialized, we first construct them all with null.
    _cellGeometry = new CellGeometry(this);
    _thumbnailCache = new ThumbnailCache;
    _model = new ThumbnailModel(this);
    _widget = new ThumbnailWidget(this);
    _painter = new ThumbnailPainter(this);
    _toolTip = new ThumbnailToolTip( _widget );

    connect( _widget, SIGNAL( showImage( const DB::ResultId& ) ),
             this, SIGNAL( showImage( const DB::ResultId& ) ) );
    connect( _widget, SIGNAL( showSelection() ),
             this, SIGNAL( showSelection() ) );
    connect( _widget, SIGNAL( fileIdUnderCursorChanged( const DB::ResultId& ) ),
             this, SIGNAL( fileIdUnderCursorChanged( const DB::ResultId&  ) ) );
    connect( _widget, SIGNAL( currentDateChanged( const QDateTime& ) ),
             this, SIGNAL( currentDateChanged( const QDateTime& ) ) );
    connect( _model, SIGNAL( selectionChanged(int) ),
             this, SIGNAL( selectionChanged(int) ) );
    connect( _model, SIGNAL( collapseAllStacksEnabled(bool ) ),
             this, SIGNAL( collapseAllStacksEnabled(bool ) ) );
    connect( _model, SIGNAL( expandAllStacksEnabled(bool) ),
             this, SIGNAL( expandAllStacksEnabled(bool ) ) );

    _instance = this;
}

QWidget* ThumbnailView::ThumbnailFacade::gui()
{
    return _widget;
}

void ThumbnailView::ThumbnailFacade::gotoDate( const DB::ImageDate& date, bool b)
{
    _widget->gotoDate( date, b );
}

void ThumbnailView::ThumbnailFacade::setCurrentItem( const DB::ResultId& id )
{
    model()->clearSelection();
    model()->select( id );
    model()->setCurrentItem(id);

    Cell cell = model()->positionForMediaId( id );
    widget()->ensureCellVisible( cell.row(), cell.col() );
}

void ThumbnailView::ThumbnailFacade::reload( bool flushCache, bool clearSelection)
{
    _widget->reload( flushCache, clearSelection );
}

DB::Result ThumbnailView::ThumbnailFacade::selection(bool keepSortOrderOfDatabase) const
{
    return _model->selection( keepSortOrderOfDatabase );
}

DB::Result ThumbnailView::ThumbnailFacade::imageList(Order order) const
{
    return _model->imageList(order);
}

DB::ResultId ThumbnailView::ThumbnailFacade::mediaIdUnderCursor() const
{
    return _widget->mediaIdUnderCursor();
}

DB::ResultId ThumbnailView::ThumbnailFacade::currentItem() const
{
    return _model->currentItem();
}

void ThumbnailView::ThumbnailFacade::setImageList(const DB::Result& list)
{
    _model->setImageList( list );
}

void ThumbnailView::ThumbnailFacade::setSortDirection( SortDirection direction )
{
    _model->setSortDirection( direction );
}

void ThumbnailView::ThumbnailFacade::selectAll()
{
    _model->selectAll();
}

void ThumbnailView::ThumbnailFacade::showToolTipsOnImages( bool on )
{
    _toolTip->setActive( on );
}

void ThumbnailView::ThumbnailFacade::repaintScreen()
{
    _widget->repaintScreen();
}

void ThumbnailView::ThumbnailFacade::toggleStackExpansion(const DB::ResultId& id)
{
    _model->toggleStackExpansion(id);
}

void ThumbnailView::ThumbnailFacade::collapseAllStacks()
{
    _model->collapseAllStacks();
}

void ThumbnailView::ThumbnailFacade::expandAllStacks()
{
    _model->expandAllStacks();
}

void ThumbnailView::ThumbnailFacade::updateDisplayModel()
{
    _model->updateDisplayModel();
}

void ThumbnailView::ThumbnailFacade::changeSingleSelection(const DB::ResultId& id)
{
    _model->changeSingleSelection(id);
}

ThumbnailView::ThumbnailModel* ThumbnailView::ThumbnailFacade::model()
{
    return _model;
}

ThumbnailView::CellGeometry* ThumbnailView::ThumbnailFacade::cellGeometry()
{
    return _cellGeometry;
}

ThumbnailView::ThumbnailWidget* ThumbnailView::ThumbnailFacade::widget()
{
    return _widget;
}

ThumbnailView::ThumbnailPainter* ThumbnailView::ThumbnailFacade::painter()
{
    return _painter;
}

ThumbnailView::ThumbnailFacade* ThumbnailView::ThumbnailFacade::instance()
{
    Q_ASSERT( _instance );
    return _instance;
}

ThumbnailView::ThumbnailCache* ThumbnailView::ThumbnailFacade::cache()
{
    return _thumbnailCache;
}
void ThumbnailView::ThumbnailFacade::slotRecreateThumbnail()
{
    Q_FOREACH( const DB::ResultId& id, model()->selectionSet() ) {
        const DB::ImageInfoPtr info = id.fetchInfo();
        const QString fileName = info->fileName(DB::AbsolutePath);
        ImageManager::Manager::instance()->removeThumbnail( fileName );

        int size = Settings::SettingsData::instance()->thumbSize();
        ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize(size,size), info->angle(), painter() );
        request->setPriority( ImageManager::BatchTask );
        ImageManager::Manager::instance()->load( request );
    }
}



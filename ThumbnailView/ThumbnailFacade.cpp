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
#include "ThumbnailFacade.h"
#include "ImageManager/ThumbnailCache.h"
#include <BackgroundJobs/HandleVideoThumbnailRequestJob.h>

#include "Settings/SettingsData.h"
#include "ThumbnailToolTip.h"
#include "ThumbnailModel.h"
#include "CellGeometry.h"
#include "ThumbnailWidget.h"

ThumbnailView::ThumbnailFacade* ThumbnailView::ThumbnailFacade::_instance = 0;
ThumbnailView::ThumbnailFacade::ThumbnailFacade()
    :_cellGeometry( new CellGeometry(this) ),
     _model( 0 ),_widget( 0 ), _toolTip( 0 )
{
    // To avoid one of the components references one of the other before it has been initialized, we first construct them all with null.
    _cellGeometry = new CellGeometry(this);
    _model = new ThumbnailModel(this);
    _widget = new ThumbnailWidget(this);
    _toolTip = new ThumbnailToolTip( _widget );

    connect( _widget, SIGNAL( showImage( const DB::FileName& ) ),
             this, SIGNAL( showImage( const DB::FileName& ) ) );
    connect( _widget, SIGNAL( showSelection() ),
             this, SIGNAL( showSelection() ) );
    connect( _widget, SIGNAL( fileIdUnderCursorChanged( const DB::FileName& ) ),
             this, SIGNAL( fileIdUnderCursorChanged( const DB::FileName&  ) ) );
    connect( _widget, SIGNAL( currentDateChanged( const QDateTime& ) ),
             this, SIGNAL( currentDateChanged( const QDateTime& ) ) );
    connect( _widget, SIGNAL( selectionCountChanged(int) ),
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

void ThumbnailView::ThumbnailFacade::setCurrentItem( const DB::FileName& fileName )
{
    widget()->setCurrentItem(fileName);
}

void ThumbnailView::ThumbnailFacade::reload( SelectionUpdateMethod method )
{
    _widget->reload( method );
}

DB::FileNameList ThumbnailView::ThumbnailFacade::selection( ThumbnailView::SelectionMode mode ) const
{
    return _widget->selection(mode);
}

DB::FileNameList ThumbnailView::ThumbnailFacade::imageList(Order order) const
{
    return _model->imageList(order);
}

DB::FileName ThumbnailView::ThumbnailFacade::mediaIdUnderCursor() const
{
    return _widget->mediaIdUnderCursor();
}

DB::FileName ThumbnailView::ThumbnailFacade::currentItem() const
{
    return _model->imageAt(_widget->currentIndex().row());
}

void ThumbnailView::ThumbnailFacade::setImageList(const DB::FileNameList& list)
{
    _model->setImageList(list);
}

void ThumbnailView::ThumbnailFacade::setSortDirection( SortDirection direction )
{
    _model->setSortDirection( direction );
}

void ThumbnailView::ThumbnailFacade::selectAll()
{
    _widget->selectAll();
}

void ThumbnailView::ThumbnailFacade::showToolTipsOnImages( bool on )
{
    _toolTip->setActive( on );
}

void ThumbnailView::ThumbnailFacade::toggleStackExpansion(const DB::FileName& fileName)
{
    _model->toggleStackExpansion(fileName);
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

void ThumbnailView::ThumbnailFacade::changeSingleSelection(const DB::FileName& fileName)
{
    _widget->changeSingleSelection(fileName);
}

ThumbnailView::ThumbnailModel* ThumbnailView::ThumbnailFacade::model()
{
    Q_ASSERT( _model );
    return _model;
}


ThumbnailView::CellGeometry* ThumbnailView::ThumbnailFacade::cellGeometry()
{
    Q_ASSERT( _cellGeometry );
    return _cellGeometry;
}

ThumbnailView::ThumbnailWidget* ThumbnailView::ThumbnailFacade::widget()
{
    Q_ASSERT( _widget );
    return _widget;
}

ThumbnailView::ThumbnailFacade* ThumbnailView::ThumbnailFacade::instance()
{
    Q_ASSERT( _instance );
    return _instance;
}

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
#include "ThumbnailModel.h"
#include "Cell.h"
#include "ThumbnailCache.h"
#include "DB/ImageDB.h"
#include "ThumbnailWidget.h"
#include "ThumbnailPainter.h"
#include "ImageManager/Manager.h"
#include "Settings/SettingsData.h"

ThumbnailView::ThumbnailModel::ThumbnailModel( ThumbnailFactory* factory)
    : ThumbnailComponent( factory ),
      _sortDirection( Settings::SettingsData::instance()->showNewestThumbnailFirst() ? NewestFirst : OldestFirst )
{
    connect( DB::ImageDB::instance(), SIGNAL( imagesDeleted( const DB::Result& ) ), this, SLOT( imagesDeletedFromDB( const DB::Result& ) ) );
}

static bool stackOrderComparator(const DB::ResultId& a, const DB::ResultId& b) {
    return a.fetchInfo()->stackOrder() < b.fetchInfo()->stackOrder();
}

void ThumbnailView::ThumbnailModel::updateDisplayModel()
{
    // FIXME: this can be probalby made obsolete by that new shiny thing in the DB

    ImageManager::Manager::instance()->stop( painter(), ImageManager::StopOnlyNonPriorityLoads );

    // Note, this can be simplified, if we make the database backend already
    // return things in the right order. Then we only need one pass while now
    // we need to go through the list two times.

    /* Extract all stacks we have first. Different stackid's might be
     * intermingled in the result so we need to know this ahead before
     * creating the display list.
     */
    typedef QList<DB::ResultId> StackList;
    typedef QMap<DB::StackID, StackList> StackMap;
    StackMap stackContents;
    Q_FOREACH(DB::ResultId id, _imageList) {
        DB::ImageInfoPtr imageInfo = id.fetchInfo();
        if (imageInfo->isStacked()) {
            DB::StackID stackid = imageInfo->stackId();
            stackContents[stackid].append(id);
        }
    }

    /*
     * All stacks need to be ordered in their stack order. We don't rely that
     * the images actually came in the order necessary.
     */
    for (StackMap::iterator it = stackContents.begin(); it != stackContents.end(); ++it) {
        qStableSort(it->begin(), it->end(), stackOrderComparator);
    }

    /* Build the final list to be displayed. That is basically the sequence
     * we got from the original, but the stacks shown with all images together
     * in the right sequence or collapsed showing only the top image.
     */
    _displayList = DB::Result();
    QSet<DB::StackID> alreadyShownStacks;
    Q_FOREACH(DB::ResultId id, _imageList) {
        DB::ImageInfoPtr imageInfo = id.fetchInfo();
        if (imageInfo->isStacked()) {
            DB::StackID stackid = imageInfo->stackId();
            if (alreadyShownStacks.contains(stackid))
                continue;
            StackMap::iterator found = stackContents.find(stackid);
            Q_ASSERT(found != stackContents.end());
            const StackList& orderedStack = *found;
            if (_expandedStacks.contains(stackid)) {
                Q_FOREACH(DB::ResultId id, orderedStack) {
                    _displayList.append(id);
                }
            } else {
                _displayList.append(orderedStack.at(0));
            }
            alreadyShownStacks.insert(stackid);
        }
        else {
            _displayList.append(id);
        }
    }

    if ( _sortDirection != OldestFirst )
        _displayList = _displayList.reversed();

    model()->updateIndexCache();

    cache()->setDisplayList(_displayList);

    emit collapseAllStacksEnabled( _expandedStacks.size() > 0);
    emit expandAllStacksEnabled( _allStacks.size() != model()->_expandedStacks.size() );
    if ( widget()->isVisible() ) {
        widget()->updateGridSize();
        widget()->repaintScreen();
    }
}

/**
 * Return the file name shown in cell (row,col) if a thumbnail is shown in this cell or null otherwise.
 */
DB::ResultId ThumbnailView::ThumbnailModel::imageAt( int row, int col ) const
{
    const int index = row * widget()->numCols() + col;
    if (index >= _displayList.size())
        return DB::ResultId::null;
    else
        return _displayList.at(index);
}

DB::ResultId ThumbnailView::ThumbnailModel::imageAt( const Cell& cell ) const
{
    return imageAt( cell.row(), cell.col() );
}

/**
 * Returns the file name shown at viewport position (x,y) if a thumbnail is shown at this position or QString::null otherwise.
 */
DB::ResultId ThumbnailView::ThumbnailModel::imageAt( const QPoint& coordinate, CoordinateSystem system ) const
{
    QPoint contentsPos = widget()->viewportToContentsAdjusted( coordinate, system );
    int col = widget()->columnAt( contentsPos.x() );
    int row = widget()->rowAt( contentsPos.y() );

    QRect cellRect = const_cast<ThumbnailWidget*>(widget())->cellGeometry( row, col );

    if ( cellRect.contains( contentsPos ) )
        return imageAt( row, col );
    else
        return DB::ResultId::null;
}

void ThumbnailView::ThumbnailModel::toggleStackExpansion(const DB::ResultId& id)
{
    DB::ImageInfoPtr imageInfo = id.fetchInfo();
    if (imageInfo) {
        DB::StackID stackid = imageInfo->stackId();
        if (_expandedStacks.contains(stackid))
            _expandedStacks.remove(stackid);
        else
            _expandedStacks.insert(stackid);
        updateDisplayModel();
    }
}

void ThumbnailView::ThumbnailModel::collapseAllStacks()
{
    _expandedStacks.clear();
    updateDisplayModel();
}

void ThumbnailView::ThumbnailModel::expandAllStacks()
{
    _expandedStacks = _allStacks;
    updateDisplayModel();
}


DB::Result ThumbnailView::ThumbnailModel::selection(bool keepSortOrderOfDatabase) const
{
    // Notice, for some reason the API here offers a list of selected
    // items, while _selectedFiles only is a set, that's why we need to
    // iterate though _imageList and insert those selected into the result.

    DB::Result images = _displayList;
    if ( keepSortOrderOfDatabase && _sortDirection == NewestFirst )
        images = images.reversed();

    DB::Result res;
    Q_FOREACH(DB::ResultId id, images) {
        if (isSelected(id))
            res.append(id);
    }
    return res;
}

void ThumbnailView::ThumbnailModel::setImageList(const DB::Result& items)
{
    _imageList = items;
    _allStacks.clear();
    Q_FOREACH(DB::ImageInfoPtr info, items.fetchInfos()) {
        if ( info && info->isStacked() )
            _allStacks << info->stackId();
    }
    // FIXME: see comments in the function -- is it really needed at all?
    // TODO(hzeller): yes so that you don't have to scroll to the page in question
    // to force loading; this is esp. painful if you have a whole bunch of new
    // images in your collection (I usually add 100 at time) - your first walk through
    // it will be slow.
    // But yeah, this needs optimization, so leaving this commented out for now.
    //generateMissingThumbnails( items );
    updateDisplayModel();
}

// TODO(hzeller) figure out if this should return the _imageList or _displayList.
DB::Result ThumbnailView::ThumbnailModel::imageList(Order order) const
{
    if ( order == SortedOrder &&  _sortDirection == NewestFirst )
        return _displayList.reversed();
    else
        return _displayList;
}

DB::ResultId ThumbnailView::ThumbnailModel::currentItem() const
{
    return _currentItem;
}

void ThumbnailView::ThumbnailModel::imagesDeletedFromDB( const DB::Result& list )
{
    Q_FOREACH( DB::ResultId id, list ) {
        _displayList.removeAll( id );
        _imageList.removeAll(id);
    }
    updateDisplayModel();
}

void ThumbnailView::ThumbnailModel::selectRange( Cell pos1, Cell pos2 )
{
    ensureCellsSorted( pos1, pos2 );

    if ( pos1.row() == pos2.row() ) {
        // This is the case where images from only one row is selected.
        for ( int col = pos1.col(); col <= pos2.col(); ++ col )
            select( pos1.row(), col );
    }
    else {
        // We know we have at least two rows.

        // first row
        for ( int col = pos1.col(); col < widget()->numCols(); ++ col )
            select( pos1.row(), col );

        // rows in between
        for ( int row = pos1.row()+1; row < pos2.row(); ++row )
            for ( int col = 0; col < widget()->numCols(); ++ col )
                select( row, col );

        // last row
        for ( int col = 0; col <= pos2.col(); ++ col )
            select( pos2.row(), col );
    }
}

void ThumbnailView::ThumbnailModel::select( const Cell& cell )
{
    select( cell.row(), cell.col() );
}

void ThumbnailView::ThumbnailModel::select( int row, int col )
{
    DB::ResultId id = imageAt( row, col );
    if ( !id.isNull() ) {
        _selectedFiles.insert( id );
        widget()->updateCell( id );
    }
    possibleEmitSelectionChanged();
}

void ThumbnailView::ThumbnailModel::clearSelection()
{
    IdSet oldSelection = _selectedFiles;
    _selectedFiles.clear();
    for( IdSet::const_iterator idIt = oldSelection.begin(); idIt != oldSelection.end(); ++idIt ) {
        widget()->updateCell( *idIt );
    }
    possibleEmitSelectionChanged();
}

void ThumbnailView::ThumbnailModel::toggleSelection( const DB::ResultId& id )
{
    if ( isSelected( id ) )
        _selectedFiles.remove( id );
    else
        _selectedFiles.insert( id );

    widget()->updateCell( id );
    possibleEmitSelectionChanged();
}

void ThumbnailView::ThumbnailModel::possibleEmitSelectionChanged()
{
    static IdSet oldSelection;
    if ( oldSelection != _selectedFiles ) {
        oldSelection = _selectedFiles;
        emit selectionChanged( _selectedFiles.count() );
    }
}

void ThumbnailView::ThumbnailModel::selectAll()
{
    _selectedFiles.clear();
    Q_FOREACH(DB::ResultId id, _displayList) {
        _selectedFiles.insert(id);
    }
    possibleEmitSelectionChanged();
    widget()->repaintScreen();
}

/**
   This very specific method will make the item specified by id selected,
   if there only are one item selected. This is used from the Viewer when
   you start it without a selection, and are going forward or backward.
*/
void ThumbnailView::ThumbnailModel::changeSingleSelection(const DB::ResultId& id)
{
    if ( _selectedFiles.size() == 1 ) {
        widget()->updateCell( *(_selectedFiles.begin()) );
        _selectedFiles.clear();
        _selectedFiles.insert( id );
        widget()->updateCell( id );
        possibleEmitSelectionChanged();
        widget()->scrollToCell( model()->positionForMediaId( id ) );
    }
}

void ThumbnailView::ThumbnailModel::ensureCellsSorted( Cell& pos1, Cell& pos2 )
{
    if ( pos2.row() < pos1.row() || ( pos2.row() == pos1.row() && pos2.col() < pos1.col() ) ) {
        Cell tmp = pos1;
        pos1 = pos2;
        pos2 = tmp;
    }
}


int ThumbnailView::ThumbnailModel::indexOf(const DB::ResultId& id ) const
{
    Q_ASSERT( !id.isNull() );
    if ( !_idToIndex.contains(id) )
        return -1;
    else
        return _idToIndex[id];
}



/**
 * return the position (row,col) for the given media id.
 */
ThumbnailView::Cell ThumbnailView::ThumbnailModel::positionForMediaId( const DB::ResultId& id ) const
{
    int index = indexOf(id);
    if ( index == -1 )
        return Cell( 0, 0 );

    int row = index / widget()->numCols();
    int col = index % widget()->numCols();
    return Cell( row, col );
}

void ThumbnailView::ThumbnailModel::updateIndexCache()
{
    _idToIndex.clear();
    int index = 0;
    Q_FOREACH(DB::ResultId id, _displayList) {
        _idToIndex[id] = index;
        ++index;
    }

}

void ThumbnailView::ThumbnailModel::setCurrentItem( const DB::ResultId& id )
{
    _currentItem = id;
}

void ThumbnailView::ThumbnailModel::setCurrentItem( const Cell& cell )
{
    _currentItem = imageAt( cell );
}

DB::ResultId ThumbnailView::ThumbnailModel::rightDropItem() const
{
    return _rightDrop;
}

void ThumbnailView::ThumbnailModel::setRightDropItem( const DB::ResultId& item )
{
    _rightDrop = item;
}

DB::ResultId ThumbnailView::ThumbnailModel::leftDropItem() const
{
    return _leftDrop;
}

void ThumbnailView::ThumbnailModel::setLeftDropItem( const DB::ResultId& item )
{
    _leftDrop = item;
}

void ThumbnailView::ThumbnailModel::setSortDirection( SortDirection direction )
{
    if ( direction == _sortDirection )
        return;

    Settings::SettingsData::instance()->setShowNewestFirst( direction == NewestFirst );
    _displayList = _displayList.reversed();
    updateIndexCache();
    cache()->setDisplayList(_displayList);
    if ( !currentItem().isNull() ) {
        const Cell cell = positionForMediaId( currentItem() );
        widget()->ensureCellVisible( cell.row(), cell.col() );
    }

    widget()->repaintScreen();

    _sortDirection = direction;
}

bool ThumbnailView::ThumbnailModel::isItemInExpandedStack( const DB::StackID& id ) const
{
    return _expandedStacks.contains(id);
}

int ThumbnailView::ThumbnailModel::imageCount() const
{
    return _displayList.size();
}

DB::ResultId ThumbnailView::ThumbnailModel::imageAt( int index ) const
{
    Q_ASSERT( index >= 0 && index < imageCount() );
    return _displayList.at(index);
}

bool ThumbnailView::ThumbnailModel::isSelected( const DB::ResultId& id ) const
{
    return _selectedFiles.contains(id);
}

void ThumbnailView::ThumbnailModel::select( const DB::ResultId& id )
{
    _selectedFiles.insert( id );
    widget()->updateCell( id );
    possibleEmitSelectionChanged();
}

ThumbnailView::IdSet ThumbnailView::ThumbnailModel::selectionSet() const
{
    return _selectedFiles;
}

void ThumbnailView::ThumbnailModel::setSelection( const IdSet& ids )
{
    IdSet changedFiles= _selectedFiles;
    changedFiles.unite( ids );

    _selectedFiles = ids;
    Q_FOREACH( const DB::ResultId& id, changedFiles )
        widget()->updateCell( id );
}


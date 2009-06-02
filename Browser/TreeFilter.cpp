#include "TreeFilter.h"
#include <QDebug>
#include "enums.h"

TreeFilter::TreeFilter( QObject* parent)
    : QSortFilterProxyModel(parent)
{

}

bool TreeFilter::filterAcceptsRow( int row, const QModelIndex & parent ) const
{
    bool match = false;
    bool openAllChildren = false;

    // If parent is open then child should be included.
    if ( _matchedMap[parent] ) {
        match = true;
        openAllChildren = true;
     }

    // check if the item itself matches
    else if ( QSortFilterProxyModel::filterAcceptsRow( row, parent ) ) {
        match = true;
        openAllChildren = true;
    }
    else {
        // Check if any children matches
        const QModelIndex myModelIndex = sourceModel()->index( row, 0, parent );
        const int childCount = sourceModel()->rowCount( myModelIndex );
        for ( int i = 0; i < childCount; ++ i ) {
            if ( filterAcceptsRow( i, myModelIndex ) ) {
                match = true;
                break;
            }
        }
    }


    _matchedMap[sourceModel()->index( row, 0, parent )] = openAllChildren;
    return match;
}

void TreeFilter::resetCache()
{
    _matchedMap.clear();
}

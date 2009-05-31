#include "TreeFilter.h"

TreeFilter::TreeFilter( QObject* parent)
    : QSortFilterProxyModel(parent)
{

}

bool TreeFilter::filterAcceptsRow( int row, const QModelIndex & parent ) const
{
    // check if the item itself matches
    if ( QSortFilterProxyModel::filterAcceptsRow( row, parent ) )
        return true;

    // Check if any children matches
    const QModelIndex myModelIndex = sourceModel()->index( row, 0, parent );
    const int childCount = sourceModel()->rowCount( myModelIndex );
    for ( int i = 0; i < childCount; ++ i ) {
        if ( filterAcceptsRow( i, myModelIndex ) )
            return true;
    }
    return false;
}

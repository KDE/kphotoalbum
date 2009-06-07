#include "FlatCategoryModel.h"
#include <klocale.h>
#include <DB/ImageDB.h>
#include <DB/MemberMap.h>
#include <KIcon>

FlatCategoryModel::FlatCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info )
    : AbstractCategoryModel( category, info )
{
    if ( hasNoneEntry() )
        _items.append( DB::ImageDB::NONE() );

    QStringList items = _category->itemsInclCategories();
    items.sort();

    for( QStringList::Iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        const QString name = *itemIt;
        const int imageCount = _images.contains(name) ? _images[name] : 0;
        const int videoCount = _videos.contains(name) ? _videos[name] : 0;

        if ( imageCount + videoCount > 0 )
            _items.append( name );
    }
}

int FlatCategoryModel::rowCount( const QModelIndex& ) const
{
    return _items.count();
}

int FlatCategoryModel::columnCount( const QModelIndex& ) const
{
    return 1;
}

OVERRIDE QModelIndex FlatCategoryModel::index( int row, int column, const QModelIndex& ) const
{
    return createIndex( row, column );
}

QModelIndex FlatCategoryModel::parent( const QModelIndex&  ) const
{
    return QModelIndex();
}

QString FlatCategoryModel::indexToName(const QModelIndex& index ) const
{
    return _items[index.row()];
}

#ifndef FLATCATEGORYMODEL_H
#define FLATCATEGORYMODEL_H
#include "AbstractCategoryModel.h"
#include <QAbstractListModel>
#include <DB/Category.h>
#include <DB/ImageSearchInfo.h>

class FlatCategoryModel :public AbstractCategoryModel
{
public:
    FlatCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info );
    OVERRIDE int rowCount( const QModelIndex& index ) const;

    OVERRIDE int columnCount( const QModelIndex& ) const;
    OVERRIDE QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    OVERRIDE QModelIndex parent ( const QModelIndex & index ) const;

    OVERRIDE QString indexToName(const QModelIndex& ) const;

private:
    QStringList _items;
};

#endif /* FLATCATEGORYMODEL_H */


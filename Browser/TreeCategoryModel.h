#ifndef TREECATEGORYMODEL_H
#define TREECATEGORYMODEL_H
#include <QAbstractItemModel>
#include "AbstractCategoryModel.h"
class Data;

namespace Browser
{

class TreeCategoryModel : public AbstractCategoryModel
{
public:
    TreeCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info );
    ~TreeCategoryModel();

    OVERRIDE int rowCount( const QModelIndex& ) const;
    OVERRIDE int columnCount( const QModelIndex& ) const;
    OVERRIDE QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    OVERRIDE QModelIndex parent ( const QModelIndex & index ) const;

    OVERRIDE QString indexToName(const QModelIndex& ) const;

private:
    bool createData( DB::CategoryItem* parentCategoryItem, Data* parent );
    Data* indexToData( const QModelIndex& index ) const;

private:
    Data* _data;
};

}

#endif /* TREECATEGORYMODEL_H */


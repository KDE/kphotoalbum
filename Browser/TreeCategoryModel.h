#ifndef TREECATEGORYMODEL_H
#define TREECATEGORYMODEL_H
#include <QAbstractItemModel>
#include "AbstractCategoryModel.h"
class Data;

namespace Browser
{

/**
 * \brief A QAbstractItemModel subclass that represent the items of a given category as a tree
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This class implements the QAbstractItemModel interface, which is
 * actually what most of the methods is about. The constructor queries
 * the category information from the back end, and builds an internal
 * data structure representing the tree. It does build its own data structure for two reasons:
 * \li The \ref DB::CategoryItem's do not have an easy way to go from child
 * to parent, something that was needed by the \ref parent method. It was
 * considered too risky to add that to the \ref DB::CategoryItem
 * data structure at the time this was implemented.
 * \li By building its own data structure it can ensure that the data is
 * not changing behind the scene, something that might have happened if
 * this class was constructed, categories was added or removed, and the
 * class was asked information abouts its data.
 */
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


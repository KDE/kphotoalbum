#ifndef ABSTRACTCATEGORYMODEL_H
#define ABSTRACTCATEGORYMODEL_H
#include <QAbstractItemModel>
#include <DB/ImageSearchInfo.h>
#include <DB/Category.h>

namespace Browser
{

/**
 * \brief
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 */
class AbstractCategoryModel :public QAbstractItemModel
{
public:
    OVERRIDE Qt::ItemFlags flags ( const QModelIndex& ) const;
    OVERRIDE QVariant data( const QModelIndex & index, int role) const;
    OVERRIDE QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

protected:
    AbstractCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info );

    bool hasNoneEntry() const;
    QString text( const QString& name ) const;
    QPixmap icon( const QString& name ) const;
    virtual QString indexToName(const QModelIndex& ) const = 0;

    DB::CategoryPtr _category;
    DB::ImageSearchInfo _info;
    QMap<QString, uint> _images;
    QMap<QString, uint> _videos;

};

}


#endif /* ABSTRACTCATEGORYMODEL_H */


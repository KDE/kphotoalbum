#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H
#include "BrowserAction.h"
#include <QStandardItemModel>
#include <DB/Category.h>
#include <DB/ImageSearchInfo.h>

class BrowserWidget;

namespace Browser {

class CategoryModel :public BrowserAction
{
public:
    CategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* browser );
    OVERRIDE void activate();
    OVERRIDE BrowserAction* generateChildAction( const QModelIndex& );

private:
    void populateBrowserWithoutHierachy( const QMap<QString, uint>& images, const QMap<QString, uint>& videos);
    bool populateBrowserWithHierachy( DB::CategoryItem* parentCategoryItem, const QMap<QString, uint>& images,
                                      const QMap<QString, uint>& videos, QStandardItem* parent );

private:
    DB::ImageSearchInfo _info;
    const DB::CategoryPtr _category;
    QStandardItemModel _model;
};

}

#endif /* CATEGORYMODEL_H */


#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H
#include "BrowserPage.h"
#include <QStandardItemModel>
#include <DB/Category.h>
#include <DB/ImageSearchInfo.h>

class FlatCategoryModel;
class BrowserWidget;

namespace Browser {

class CategoryPage :public BrowserPage
{
public:
    CategoryPage( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* browser );
    OVERRIDE void activate();
    OVERRIDE BrowserPage* activateChild( const QModelIndex& );
    OVERRIDE DB::Category::ViewType viewType() const;

    DB::CategoryPtr category() const;

private:
    const DB::CategoryPtr _category;
    QAbstractItemModel* _model;
};

}

#endif /* CATEGORYPAGE_H */


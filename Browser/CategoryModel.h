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
    OVERRIDE DB::Category::ViewType viewType() const;

    const DB::CategoryPtr category() const;

private:
    void populateModel();
    void populateBrowserWithoutHierachy( const QMap<QString, uint>& images, const QMap<QString, uint>& videos);
    bool populateBrowserWithHierachy( DB::CategoryItem* parentCategoryItem, const QMap<QString, uint>& images,
                                      const QMap<QString, uint>& videos, QStandardItem* parent );
    QList<QStandardItem*> createItem( const QString& name, int imageCount, int videoCount );
    QString text( const QString& name );
    QPixmap icon( const QString& name );

private:
    const DB::CategoryPtr _category;
    QStandardItemModel _model;
};

}

#endif /* CATEGORYMODEL_H */


#include "CategoryModel.h"
#include <DB/ImageDB.h>
#include <QDebug>
#include <Settings/SettingsData.h>
#include "OverviewModel.h"
#include "BrowserWidget.h"
#include <DB/CategoryItem.h>
const int ItemNameRole = Qt::UserRole + 1;

Browser::CategoryModel::CategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : BrowserAction( browser ),_info(info), _category( category )
{
    QMap<QString, uint> images = DB::ImageDB::instance()->classify( _info, category->name(), DB::Image );
    QMap<QString, uint> videos = DB::ImageDB::instance()->classify( _info, category->name(), DB::Video );

#ifdef KDAB_TEMPORARILY_REMOVED
    // Code from Browser::TypeFolderAction::action
    DB::CategoryItemPtr item = category->itemsCategories();

    // Add the none option to the end
    int imageCount = images[DB::ImageDB::NONE()];
    int videoCount = videos[DB::ImageDB::NONE()];
    if ( imageCount + videoCount != 0 )
        factory->createItem( new ContentFolder( _category, DB::ImageDB::NONE(), DB::MediaCount( imageCount, videoCount ),
                                                _info, _browser ), 0 );
#endif //KDAB_TEMPORARILY_REMOVED


#ifdef KDAB_TEMPORARILY_REMOVED
    populateBrowserWithoutHierachy(images,videos);
#endif //KDAB_TEMPORARILY_REMOVED


    populateBrowserWithHierachy( _category->itemsCategories().data(), images, videos, 0 );
}

void Browser::CategoryModel::activate()
{
    browser()->setModel( &_model );
}

Browser::BrowserAction* Browser::CategoryModel::generateChildAction( const QModelIndex& index )
{
    const QString name = _model.data( index, ItemNameRole ).value<QString>();
    DB::ImageSearchInfo info = _info;

    info.addAnd( _category->name(), name );
    return new Browser::OverviewModel( info, browser() );
}

void Browser::CategoryModel::populateBrowserWithoutHierachy( const QMap<QString, uint>& images, const QMap<QString, uint>& videos)
{
    QStringList items = _category->itemsInclCategories();
    items.sort();

    for( QStringList::Iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        const QString name = *itemIt;
        const int imageCnt = images.contains(name) ? images[name] : 0;
        const int videoCnt = videos.contains(name) ? videos[name] : 0;

        if ( imageCnt + videoCnt > 0 ) {
            QStandardItem* item = new QStandardItem( name );//QString::fromLatin1( "%1 (%2/%3)").arg(name).arg(imageCnt).arg(videoCnt) );
            item->setIcon( Settings::SettingsData::instance()->categoryImage( _category->name(), name, 100 ) ); //_category->thumbnailSize() ) );
            item->setData( name, ItemNameRole );
            _model.appendRow( item );
        }
    }
}

bool Browser::CategoryModel::populateBrowserWithHierachy( DB::CategoryItem* parentCategoryItem, const QMap<QString, uint>& images,
                                                          const QMap<QString, uint>& videos, QStandardItem* parent )
{
    const QString name = parentCategoryItem->_name;
    const int imageCtn = images.contains(name) ? images[name] : 0;
    const int videoCtn = videos.contains(name) ? videos[name] : 0;

    QStandardItem* item = 0;
    if ( !parentCategoryItem->_isTop ) {
        item = new QStandardItem( name );//QString::fromLatin1( "%1 (%2/%3)").arg(name).arg(imageCnt).arg(videoCnt) );
        item->setIcon( Settings::SettingsData::instance()->categoryImage( _category->name(), name, 100 ) ); //_category->thumbnailSize() ) );
        item->setData( name, ItemNameRole );

        if ( parent )
            parent->appendRow( item );
        else
            _model.appendRow( item );
    }

    bool anyItems = imageCtn != 0 || videoCtn != 0;

    for( Q3ValueList<DB::CategoryItem*>::ConstIterator subCategoryIt = parentCategoryItem->_subcategories.constBegin();
         subCategoryIt != parentCategoryItem->_subcategories.constEnd(); ++subCategoryIt ) {
        anyItems = populateBrowserWithHierachy( *subCategoryIt, images, videos, item ) || anyItems;
    }

    if ( !anyItems ) {
        delete item;
    }

    return anyItems;
}

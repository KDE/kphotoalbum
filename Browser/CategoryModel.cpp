#include "CategoryModel.h"
#include <QDebug>
#include <Settings/SettingsData.h>
#include "OverviewModel.h"
#include "BrowserWidget.h"

Browser::CategoryModel::CategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : BrowserAction( browser ),_info(info), _category( category )
{
#ifdef KDAB_TEMPORARILY_REMOVED
    // Code from Browser::TypeFolderAction::action
    QMap<QString, uint> images = DB::ImageDB::instance()->classify( _info, category->name(), DB::Image );
    QMap<QString, uint> videos = DB::ImageDB::instance()->classify( _info, category->name(), DB::Video );

    DB::CategoryItemPtr item = category->itemsCategories();

    // Add the none option to the end
    int imageCount = images[DB::ImageDB::NONE()];
    int videoCount = videos[DB::ImageDB::NONE()];
    if ( imageCount + videoCount != 0 )
        factory->createItem( new ContentFolder( _category, DB::ImageDB::NONE(), DB::MediaCount( imageCount, videoCount ),
                                                _info, _browser ), 0 );
#endif //KDAB_TEMPORARILY_REMOVED


    // code from Browser::TypeFolderAction::populateBrowserWithoutHierachy
    QStringList items = category->itemsInclCategories();
    _model.setColumnCount(1);
    _model.setRowCount( items.count() );

    items.sort();

    int row = 0;
    for( QStringList::Iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt, ++row ) {
        const QString name = *itemIt;
        const QModelIndex index = _model.index( row, 0 );

        _model.setData( index, name, Qt::DisplayRole );
        _model.setData( index, Settings::SettingsData::instance()->categoryImage( _category->name(), name, _category->thumbnailSize() ), Qt::DecorationRole );

#ifdef KDAB_TEMPORARILY_REMOVED
        int imageCtn = images.contains(name) ? images[name] : 0;
        int videoCtn = videos.contains(name) ? videos[name] : 0;
        if ( imageCtn + videoCtn > 0 ) {

            factory->createItem( new Browser::ContentFolder( _category, name, DB::MediaCount( imageCtn, videoCtn ),
                                                             _info, _browser ), 0 );
        }

#endif //KDAB_TEMPORARILY_REMOVED
    }


}

void Browser::CategoryModel::activate()
{
    browser()->setModel( &_model );
}

Browser::BrowserAction* Browser::CategoryModel::generateChildAction( const QModelIndex& index )
{
    const QString name = _model.data( index, Qt::DisplayRole ).value<QString>();
    DB::ImageSearchInfo info = _info;

    info.addAnd( _category->name(), name );
    return new Browser::OverviewModel( info, browser() );
}


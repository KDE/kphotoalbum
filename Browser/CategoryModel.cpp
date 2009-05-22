#include "CategoryModel.h"
#include <DB/MemberMap.h>
#include <klocale.h>
#include <DB/ImageDB.h>
#include <QDebug>
#include <Settings/SettingsData.h>
#include "OverviewModel.h"
#include "BrowserWidget.h"
#include <DB/CategoryItem.h>
#include <KIcon>

const int ItemNameRole = Qt::UserRole + 1;

Browser::CategoryModel::CategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : BrowserAction( browser ),_info(info), _category( category )
{
}

void Browser::CategoryModel::activate()
{
    populateModel();
    browser()->setModel( &_model );
}

Browser::BrowserAction* Browser::CategoryModel::generateChildAction( const QModelIndex& index )
{
    const QString name = _model.data( index, ItemNameRole ).value<QString>();
    DB::ImageSearchInfo info = _info;

    info.addAnd( _category->name(), name );
    return new Browser::OverviewModel( info, browser() );
}

void Browser::CategoryModel::populateModel()
{
    _model.clear();
    QMap<QString, uint> images = DB::ImageDB::instance()->classify( _info, _category->name(), DB::Image );
    QMap<QString, uint> videos = DB::ImageDB::instance()->classify( _info, _category->name(), DB::Video );

    // Add the none option to the end
    int imageCount = images[DB::ImageDB::NONE()];
    int videoCount = videos[DB::ImageDB::NONE()];
    if ( imageCount + videoCount != 0 )
        _model.appendRow( createItem( DB::ImageDB::NONE() ) );

    if ( _category->viewType() == DB::Category::ListView || _category->viewType() == DB::Category::ThumbedListView )
        populateBrowserWithHierachy( _category->itemsCategories().data(), images, videos, 0 );
    else
        populateBrowserWithoutHierachy(images,videos);
}

void Browser::CategoryModel::populateBrowserWithoutHierachy( const QMap<QString, uint>& images, const QMap<QString, uint>& videos)
{
    QStringList items = _category->itemsInclCategories();
    items.sort();

    for( QStringList::Iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        const QString name = *itemIt;
        const int imageCnt = images.contains(name) ? images[name] : 0;
        const int videoCnt = videos.contains(name) ? videos[name] : 0;

        if ( imageCnt + videoCnt > 0 )
            _model.appendRow( createItem( name ) );
    }
}

bool Browser::CategoryModel::populateBrowserWithHierachy( DB::CategoryItem* parentCategoryItem, const QMap<QString, uint>& images,
                                                          const QMap<QString, uint>& videos, QStandardItem* parent )
{
    const QString name = parentCategoryItem->_name;
    const int imageCtn = images.contains(name) ? images[name] : 0;
    const int videoCtn = videos.contains(name) ? videos[name] : 0;

    QStandardItem* item = 0;
    if ( !parentCategoryItem->_isTop )
        item = createItem( name );

    bool anyItems = imageCtn != 0 || videoCtn != 0;

    for( Q3ValueList<DB::CategoryItem*>::ConstIterator subCategoryIt = parentCategoryItem->_subcategories.constBegin();
         subCategoryIt != parentCategoryItem->_subcategories.constEnd(); ++subCategoryIt ) {
        anyItems = populateBrowserWithHierachy( *subCategoryIt, images, videos, item ) || anyItems;
    }

    if ( anyItems && item ) {
        if ( parent )
            parent->appendRow( item );
        else
            _model.appendRow( item );
    }
    else
        delete item;

    return anyItems;
}

const DB::CategoryPtr Browser::CategoryModel::category() const
{
    return _category;
}

DB::Category::ViewType Browser::CategoryModel::viewType() const
{
    return _category->viewType();
}

QStandardItem* Browser::CategoryModel::createItem( const QString& name )
{
    QStandardItem* item = new QStandardItem( text(name) );//QString::fromLatin1( "%1 (%2/%3)").arg(name).arg(imageCnt).arg(videoCnt) );
    item->setIcon( icon(name) );
    item->setData( name, ItemNameRole );

    return item;
}

QString Browser::CategoryModel::text( const QString& name )
{
    if ( name == DB::ImageDB::NONE() ) {
        if ( _info.option(_category->name()).length() == 0 )
            return i18n( "None" );
        else
            return i18n( "No other" );
    }
    else if ( name == QString::fromLatin1( "Video" ) )
        return i18n("Video");
    else if ( name == QString::fromLatin1( "Image" ) )
        return i18n("Image");

    else {
        if ( _category->name() == QString::fromLatin1( "Folder" ) ) {
            QRegExp rx( QString::fromLatin1( "(.*/)(.*)$") );
            QString value = name;
            value.replace( rx, QString::fromLatin1("\\2") );
            return value;
        } else {
            return name;
        }
    }
}

QPixmap Browser::CategoryModel::icon( const QString& name )
{
    if ( _category->viewType() == DB::Category::ListView || _category->viewType() == DB::Category::IconView ) {
        if ( DB::ImageDB::instance()->memberMap().isGroup( _category->name(), name ) )
            return KIcon( QString::fromLatin1( "folder_image" ) ).pixmap(22);
        else {
            return _category->icon();
        }
    }
    else
        return Settings::SettingsData::instance()->categoryImage( _category->name(), name, _category->thumbnailSize() );
}



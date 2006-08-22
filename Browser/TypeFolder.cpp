/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "TypeFolder.h"
#include "DB/ImageDB.h"
#include "ContentFolder.h"
#include <klocale.h>
#include "BrowserItemFactory.h"
#include "DB/CategoryCollection.h"
#include <DB/CategoryItem.h>

Browser::TypeFolder::TypeFolder( const QString& category, const DB::ImageSearchInfo& info, BrowserWidget* parent )
    :Folder( info, parent ), _category ( category )
{
    QMap<QString, int> images = DB::ImageDB::instance()->classify( _info, _category, DB::Image );
    QMap<QString, int> videos = DB::ImageDB::instance()->classify( _info, _category, DB::Video );
    DB::MediaCount count( images.count(), videos.count() );
    setCount( count );
    if ( count.total() <= 1 )
        setEnabled( false );
}

QPixmap Browser::TypeFolder::pixmap()
{
    return DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->icon();
}

QString Browser::TypeFolder::text() const
{
    return DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->text();
}

Browser::FolderAction* Browser::TypeFolder::action( bool /* ctrlDown */ )
{
    return new TypeFolderAction( _category, _info, _browser );
}

Browser::TypeFolderAction::TypeFolderAction( const QString& category, const DB::ImageSearchInfo& info,
                                    BrowserWidget* browser )
    :FolderAction( info, browser ), _category( category )
{
}


bool Browser::TypeFolderAction::populateBrowserWithHierachy( DB::CategoryItem* parentCategoryItem, const QMap<QString, int>& images,
                                                 const QMap<QString, int>& videos, BrowserItemFactory* factory,
                                                 BrowserItem* parentBrowserItem )
{
    QString name = parentCategoryItem->_name;
    int imageCtn = images.contains(name) ? images[name] : 0;
    int videoCtn = videos.contains(name) ? videos[name] : 0;

    BrowserItem* item = 0;
    if ( !parentCategoryItem->_isTop )
        item = factory->createItem( new Browser::ContentFolder( _category, name, DB::MediaCount( imageCtn, videoCtn ),
                                                                _info, _browser ), parentBrowserItem );

    bool anyItems = imageCtn != 0 || videoCtn != 0;

    for( QValueList<DB::CategoryItem*>::ConstIterator subCategoryIt = parentCategoryItem->_subcategories.begin();
         subCategoryIt != parentCategoryItem->_subcategories.end(); ++subCategoryIt ) {
        anyItems = populateBrowserWithHierachy( *subCategoryIt, images, videos, factory, item ) || anyItems;
    }

    if ( !anyItems ) {
        delete item;
    }

    return anyItems;
}

void Browser::TypeFolderAction::populateBrowserWithoutHierachy( const QMap<QString, int>& images,
                                                                const QMap<QString, int>& videos, BrowserItemFactory* factory )
{
    QStringList items = DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->itemsInclCategories();
    items.sort();

    for( QStringList::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        QString name = *itemIt;
        int imageCtn = images.contains(name) ? images[name] : 0;
        int videoCtn = videos.contains(name) ? videos[name] : 0;
        factory->createItem( new Browser::ContentFolder( _category, name, DB::MediaCount( imageCtn, videoCtn ),
                                                         _info, _browser ), 0 );
    }
}



void Browser::TypeFolderAction::action( BrowserItemFactory* factory )
{
    _browser->clear();

    QMap<QString, int> images = DB::ImageDB::instance()->classify( _info, _category, DB::Image );
    QMap<QString, int> videos = DB::ImageDB::instance()->classify( _info, _category, DB::Video );

    DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( _category );
    KSharedPtr<DB::CategoryItem> item = category->itemsCategories();

    // Add the none option to the end
    int imageCount = images[DB::ImageDB::NONE()];
    int videoCount = videos[DB::ImageDB::NONE()];
    if ( imageCount + videoCount != 0 )
        factory->createItem( new ContentFolder( _category, DB::ImageDB::NONE(), DB::MediaCount( imageCount, videoCount ),
                                                _info, _browser ), 0 );

    if ( factory->supportsHierarchy() )
        populateBrowserWithHierachy( item, images, videos, factory, 0 );
    else
        populateBrowserWithoutHierachy( images, videos, factory );
}

QString Browser::TypeFolderAction::title() const
{
    return DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->text();
}

QString Browser::TypeFolderAction::category() const
{
    return _category;
}

QString Browser::TypeFolder::imagesLabel() const
{
    return i18n("1 Category", "%n Categories", _count.images());
}

QString Browser::TypeFolder::videosLabel() const
{
    return i18n("1 Category", "%n Categories", _count.videos());
}

bool Browser::TypeFolderAction::contentView() const
{
    return ( !DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->isSpecialCategory() );
}


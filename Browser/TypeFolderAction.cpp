/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#include "TypeFolderAction.h"
#include "DB/ImageDB.h"
#include "ContentFolder.h"
#include "BrowserItemFactory.h"
#include "DB/Category.h"
#include <DB/CategoryItem.h>

Browser::TypeFolderAction::TypeFolderAction( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info,
                                             BrowserWidget* browser )
    :FolderAction( info, browser ), _category( category )

{
}

bool Browser::TypeFolderAction::populateBrowserWithHierachy( DB::CategoryItem* parentCategoryItem, const QMap<QString, uint>& images,
                                                 const QMap<QString, uint>& videos, BrowserItemFactory* factory,
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

void Browser::TypeFolderAction::populateBrowserWithoutHierachy( const QMap<QString, uint>& images,
                                                                const QMap<QString, uint>& videos, BrowserItemFactory* factory )
{
    QStringList items = _category->itemsInclCategories();
    items.sort();

    for( QStringList::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        QString name = *itemIt;
        int imageCtn = images.contains(name) ? images[name] : 0;
        int videoCtn = videos.contains(name) ? videos[name] : 0;
        if ( imageCtn + videoCtn > 0 )
            factory->createItem( new Browser::ContentFolder( _category, name, DB::MediaCount( imageCtn, videoCtn ),
                                                             _info, _browser ), 0 );
    }
}

void Browser::TypeFolderAction::action( BrowserItemFactory* factory )
{
    _browser->clear();

    QMap<QString, uint> images = DB::ImageDB::instance()->classify( _info, _category->name(), DB::Image );
    QMap<QString, uint> videos = DB::ImageDB::instance()->classify( _info, _category->name(), DB::Video );

    KSharedPtr<DB::CategoryItem> item = _category->itemsCategories();

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
    return _category->text();
}

DB::CategoryPtr Browser::TypeFolderAction::category() const
{
    return _category;
}

bool Browser::TypeFolderAction::contentView() const
{
    return true;
}

DB::Category::ViewType Browser::TypeFolderAction::viewType() const
{
    return _category->viewType();
}



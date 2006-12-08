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
#include "Category.h"
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include "DB/ImageDB.h"
#include "DB/MemberMap.h"
#include "CategoryItem.h"

QPixmap DB::Category::icon( int size ) const
{
    return KGlobal::iconLoader()->loadIcon( iconName(), KIcon::Desktop, size );
}


/**
   If one person from say Denmark sends a database to a person from say germany, then the title of
   People, Places, and Keywords will still be translated correct, when this function is used.
*/
QString DB::Category::text() const
{
    if (standardCategories().contains( name() ) )
        return standardCategories()[name()];
    else
        return name();
}

QStringList DB::Category::itemsInclCategories() const
{
    // values including member groups

    QStringList items = this->items();

    // add the groups to the list too, but only if the group is not there already, which will be the case
    // if it has ever been selected once.
    QStringList groups = DB::ImageDB::instance()->memberMap().groups( name() );
    for( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
        if ( ! items.contains(  *it ) )
            items << *it ;
    };

    return items;
}

DB::CategoryItem* createItem( const QString& categoryName, const QString& itemName, StringSet handledItems,
                              QMap<QString,DB::CategoryItem*>& categoryItems,
                              QMap<QString, DB::CategoryItem*>& potentialToplevelItems )
{
    handledItems.insert( itemName );
    DB::CategoryItem* result = new DB::CategoryItem( itemName );

    const QStringList members = DB::ImageDB::instance()->memberMap().members( categoryName, itemName, false );
    for( QStringList::ConstIterator memberIt = members.begin(); memberIt != members.end(); ++memberIt ) {
        if ( !handledItems.contains( *memberIt ) ) {
            DB::CategoryItem* child;
            if ( categoryItems.contains( *memberIt ) )
                child = categoryItems[*memberIt]->clone();
            else
                child = createItem( categoryName, *memberIt, handledItems, categoryItems, potentialToplevelItems );

            potentialToplevelItems.remove( *memberIt );
            result->_subcategories.append( child );
        }
    }

    categoryItems.insert( itemName, result );
    return result;
}

KSharedPtr<DB::CategoryItem> DB::Category::itemsCategories() const
{
    const MemberMap& map = ImageDB::instance()->memberMap();
    const QStringList groups = map.groups( name() );

    QMap<QString, DB::CategoryItem*> categoryItems;
    QMap<QString, DB::CategoryItem*> potentialToplevelItems;

    for( QStringList::ConstIterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
        if ( !categoryItems.contains( *groupIt ) ) {
            StringSet handledItems;
            DB::CategoryItem* child = createItem( name(), *groupIt, handledItems, categoryItems, potentialToplevelItems );
            potentialToplevelItems.insert( *groupIt, child );
        }
    }


    CategoryItem* result = new CategoryItem( QString::fromLatin1("top"), true );
    for( QMap<QString,DB::CategoryItem*>::Iterator toplevelIt = potentialToplevelItems.begin(); toplevelIt != potentialToplevelItems.end(); ++toplevelIt ) {
        result->_subcategories.append( *toplevelIt );
    }

    // Add items not found yet.
    QStringList elms = items();
    for( QStringList::ConstIterator elmIt = elms.begin(); elmIt != elms.end(); ++elmIt ) {
        if ( !categoryItems.contains( *elmIt ) )
            result->_subcategories.append( new DB::CategoryItem( *elmIt ) );
    }

    return result;
}

QMap<QString,QString> DB::Category::standardCategories()
{
    static QMap<QString,QString> map;
    if ( map.isEmpty() ) {
        map.insert( QString::fromLatin1( "People" ), i18n("People") );
        map.insert( QString::fromLatin1( "Places" ), i18n("Places") );
        map.insert( QString::fromLatin1( "Keywords" ),  i18n("Keywords") );
        map.insert( QString::fromLatin1( "Folder" ),  i18n("Folder") );
        map.insert( QString::fromLatin1( "Tokens" ),  i18n("Tokens") );
        map.insert( QString::fromLatin1( "Media Type" ),  i18n("Media Type") );

        // Needed for compatibility with index.xml files from older versions of KPA.
        map.insert( QString::fromLatin1( "Persons" ), i18n("People") );
        map.insert( QString::fromLatin1( "Locations" ), i18n("Places") );
    }
    return map;
}

#include "Category.moc"

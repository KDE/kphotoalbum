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
#include "XMLCategory.h"
#include "DB/ImageDB.h"
#include "DB/MemberMap.h"
#include <DB/ImageDB.h>
#include "Utilities/List.h"

XMLDB::XMLCategory::XMLCategory( const QString& name, const QString& icon, ViewType type, int thumbnailSize, bool show )
    : _name( name ), _icon( icon ), _show( show ), _type( type ), _thumbnailSize( thumbnailSize ), _isSpecial(false), _shouldSave( true )
{
}

QString XMLDB::XMLCategory::name() const
{
    return _name;
}

void XMLDB::XMLCategory::setName( const QString& name )
{
    _name = name;
}

QString XMLDB::XMLCategory::iconName() const
{
    return _icon;
}

void XMLDB::XMLCategory::setIconName( const QString& name )
{
    _icon = name;
    emit changed();
}

void XMLDB::XMLCategory::setViewType( ViewType type )
{
    _type = type;
    emit changed();
}

XMLDB::XMLCategory::ViewType XMLDB::XMLCategory::viewType() const
{
    return _type;
}

void XMLDB::XMLCategory::setDoShow( bool b )
{
    _show = b;
    emit changed();
}

bool XMLDB::XMLCategory::doShow() const
{
    return _show;
}

void XMLDB::XMLCategory::setSpecialCategory( bool b )
{
    _isSpecial = b;
}

bool XMLDB::XMLCategory::isSpecialCategory() const
{
    return _isSpecial;
}

void XMLDB::XMLCategory::addOrReorderItems( const QStringList& items )
{
    _items = Utilities::mergeListsUniqly(items, _items);
}

void XMLDB::XMLCategory::setItems( const QStringList& items )
{
    _items = items;
}

void XMLDB::XMLCategory::removeItem( const QString& item )
{
    _items.remove( item );
    emit itemRemoved( item );
}

void XMLDB::XMLCategory::renameItem( const QString& oldValue, const QString& newValue )
{
    _items.remove( oldValue );
    addItem( newValue );
    emit itemRenamed( oldValue, newValue );
}

void XMLDB::XMLCategory::addItem( const QString& item )
{
    if (_items.contains( item ) )
        _items.remove( item );
    _items.prepend( item );
}

QStringList XMLDB::XMLCategory::items() const
{
    return _items;
}

int XMLDB::XMLCategory::idForName( const QString& name ) const
{
    return _idMap[name];
}

void XMLDB::XMLCategory::initIdMap()
{
    int i = 0;
    _idMap.clear();
    for( QStringList::Iterator it = _items.begin(); it != _items.end(); ++it ) {
        _idMap.insert( *it, ++i );
    }

    QStringList groups = DB::ImageDB::instance()->memberMap().groups(_name);
    for( QStringList::ConstIterator groupIt = groups.begin(); groupIt != groups.end(); ++groupIt ) {
        if ( !_idMap.contains( *groupIt ) )
            _idMap.insert( *groupIt, ++i );
    }
}

void XMLDB::XMLCategory::setIdMapping( const QString& name, int id )
{
    _nameMap.insert( id, name );
}

QString XMLDB::XMLCategory::nameForId( int id ) const
{
    return _nameMap[id];
}

void XMLDB::XMLCategory::setThumbnailSize( int size )
{
    _thumbnailSize = size;
    emit changed();
}

int XMLDB::XMLCategory::thumbnailSize() const
{
    return _thumbnailSize;
}

bool XMLDB::XMLCategory::shouldSave()
{
    return _shouldSave;
}

void XMLDB::XMLCategory::setShouldSave( bool b)
{
    _shouldSave = b;
}

#include "XMLCategory.moc"

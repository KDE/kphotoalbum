/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "XMLCategory.h"
#include "DB/ImageDB.h"
#include "DB/MemberMap.h"
#include "Utilities/List.h"
#include <kdebug.h>

XMLDB::XMLCategory::XMLCategory( const QString& name, const QString& icon, ViewType type, int thumbnailSize, bool show, bool positionable )
    : m_name( name ), m_icon( icon ), m_show( show ), m_type( type ), m_thumbnailSize( thumbnailSize ), m_positionable ( positionable ), m_isSpecial(false), m_shouldSave( true )
{
}

QString XMLDB::XMLCategory::name() const
{
    return m_name;
}

void XMLDB::XMLCategory::setName( const QString& name )
{
    m_name = name;
}

void XMLDB::XMLCategory::setPositionable( bool positionable )
{
    if ( positionable != m_positionable )
    {
        m_positionable = positionable;
        emit changed();
    }
}

bool XMLDB::XMLCategory::positionable() const
{
    return m_positionable;
}

QString XMLDB::XMLCategory::iconName() const
{
    return m_icon;
}

void XMLDB::XMLCategory::setIconName( const QString& name )
{
    m_icon = name;
    emit changed();
}

void XMLDB::XMLCategory::setViewType( ViewType type )
{
    m_type = type;
    emit changed();
}

XMLDB::XMLCategory::ViewType XMLDB::XMLCategory::viewType() const
{
    return m_type;
}

void XMLDB::XMLCategory::setDoShow( bool b )
{
    m_show = b;
    emit changed();
}

bool XMLDB::XMLCategory::doShow() const
{
    return m_show;
}

void XMLDB::XMLCategory::setSpecialCategory( bool b )
{
    m_isSpecial = b;
}

bool XMLDB::XMLCategory::isSpecialCategory() const
{
    return m_isSpecial;
}

void XMLDB::XMLCategory::addOrReorderItems( const QStringList& items )
{
    m_items = Utilities::mergeListsUniqly(items, m_items);
}

void XMLDB::XMLCategory::setItems( const QStringList& items )
{
    m_items = items;
}

void XMLDB::XMLCategory::removeItem( const QString& item )
{
    m_items.removeAll( item );
    emit itemRemoved( item );
}

void XMLDB::XMLCategory::renameItem( const QString& oldValue, const QString& newValue )
{
    m_items.removeAll( oldValue );
    addItem( newValue );
    emit itemRenamed( oldValue, newValue );
}

void XMLDB::XMLCategory::addItem( const QString& item )
{
    if (m_items.contains( item ) )
        m_items.removeAll( item );
    m_items.prepend( item );
}

QStringList XMLDB::XMLCategory::items() const
{
    return m_items;
}

int XMLDB::XMLCategory::idForName( const QString& name ) const
{
    return m_idMap[name];
}

void XMLDB::XMLCategory::initIdMap()
{
    int i = 0;
    m_idMap.clear();
    for( QStringList::Iterator it = m_items.begin(); it != m_items.end(); ++it ) {
        m_idMap.insert( *it, ++i );
    }

    QStringList groups = DB::ImageDB::instance()->memberMap().groups(m_name);
    for( QStringList::ConstIterator groupIt = groups.constBegin(); groupIt != groups.constEnd(); ++groupIt ) {
        if ( !m_idMap.contains( *groupIt ) )
            m_idMap.insert( *groupIt, ++i );
    }
}

void XMLDB::XMLCategory::setIdMapping( const QString& name, int id )
{
    m_nameMap.insert( id, name );
}

QString XMLDB::XMLCategory::nameForId( int id ) const
{
    return m_nameMap[id];
}

void XMLDB::XMLCategory::setThumbnailSize( int size )
{
    m_thumbnailSize = size;
    emit changed();
}

int XMLDB::XMLCategory::thumbnailSize() const
{
    return m_thumbnailSize;
}

bool XMLDB::XMLCategory::shouldSave()
{
    return m_shouldSave;
}

void XMLDB::XMLCategory::setShouldSave( bool b)
{
    m_shouldSave = b;
}

void XMLDB::XMLCategory::setBirthDate(const QString &item, const QDate &birthDate)
{
    m_birthDates.insert(item,birthDate);
}

QDate XMLDB::XMLCategory::birthDate(const QString &item) const
{
    return m_birthDates[item];
}

#include "XMLCategory.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:

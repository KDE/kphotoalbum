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
#include "DB/ImageDB.h"
#include "XMLCategoryCollection.h"
#include "XMLCategory.h"

DB::CategoryPtr XMLDB::XMLCategoryCollection::categoryForName( const QString& name ) const
{
    for( QValueList<DB::CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name )
            return *it;
    }
    return 0;
}

void XMLDB::XMLCategoryCollection::addCategory( DB::Category* category )
{
    _categories.append( category );
    connect( category, SIGNAL( changed() ), this, SIGNAL( categoryCollectionChanged() ) );
    connect( category, SIGNAL( itemRemoved( const QString& ) ), this, SLOT( itemRemoved( const QString& ) ) );
    connect( category, SIGNAL( itemRenamed( const QString&, const QString& ) ), this, SLOT( itemRenamed( const QString&, const QString& ) ) );
    emit categoryCollectionChanged();
}

QStringList XMLDB::XMLCategoryCollection::categoryNames() const
{
    QStringList res;
    for( QValueList<DB::CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        res.append( (*it)->name() );
    }
    return res;
}

void XMLDB::XMLCategoryCollection::removeCategory( const QString& name )
{
    for( QValueList<DB::CategoryPtr>::Iterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name ) {
            _categories.remove(it);
            emit categoryCollectionChanged();
            return;
        }
    }
    Q_ASSERT( false );
}

void XMLDB::XMLCategoryCollection::rename( const QString& oldName, const QString& newName )
{
    categoryForName(oldName)->setName(newName);
    DB::ImageDB::instance()->renameCategory( oldName, newName );
    emit categoryCollectionChanged();

}

QValueList<DB::CategoryPtr> XMLDB::XMLCategoryCollection::categories() const
{
    return _categories;
}

void XMLDB::XMLCategoryCollection::addCategory( const QString& text, const QString& icon,
                                                DB::Category::ViewType type, int thumbnailSize, bool show )
{
    addCategory( new XMLCategory( text, icon, type, thumbnailSize, show ) );
}

void XMLDB::XMLCategoryCollection::initIdMap()
{
    for( QValueList<DB::CategoryPtr>::Iterator it = _categories.begin(); it != _categories.end(); ++it )
        static_cast<XMLCategory*>((*it).data())->initIdMap();
}

#include "XMLCategoryCollection.moc"

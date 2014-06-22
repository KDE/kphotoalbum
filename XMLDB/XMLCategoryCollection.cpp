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
#include "XMLCategoryCollection.h"
#include "DB/ImageDB.h"
#include "XMLCategory.h"
#include <QList>
#include <kdebug.h>

DB::CategoryPtr XMLDB::XMLCategoryCollection::categoryForName( const QString& name ) const
{
    for( QList<DB::CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name || (*it)->text() == name )
            return *it;
    }
    return DB::CategoryPtr();
}

void XMLDB::XMLCategoryCollection::addCategory( DB::CategoryPtr category )
{
    _categories.append( category );
    connect( category.data(), SIGNAL(changed()), this, SIGNAL(categoryCollectionChanged()) );
    connect( category.data(), SIGNAL(itemRemoved(QString)), this, SLOT(itemRemoved(QString)) );
    connect( category.data(), SIGNAL(itemRenamed(QString,QString)), this, SLOT(itemRenamed(QString,QString)) );
    emit categoryCollectionChanged();
}

QStringList XMLDB::XMLCategoryCollection::categoryNames() const
{
    QStringList res;
    for( QList<DB::CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        res.append( (*it)->name() );
    }
    return res;
}

QStringList XMLDB::XMLCategoryCollection::categoryTexts() const
{
    QStringList res;
    for( QList<DB::CategoryPtr>::ConstIterator it = _categories.begin(); it != _categories.end(); ++it ) {
        res.append( (*it)->text() );
    }
    return res;
}

void XMLDB::XMLCategoryCollection::removeCategory( const QString& name )
{
    for( QList<DB::CategoryPtr>::Iterator it = _categories.begin(); it != _categories.end(); ++it ) {
        if ( (*it)->name() == name ) {
            _categories.erase(it);
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

QList<DB::CategoryPtr> XMLDB::XMLCategoryCollection::categories() const
{
    return _categories;
}

void XMLDB::XMLCategoryCollection::addCategory( const QString& text, const QString& icon,
                                                DB::Category::ViewType type, int thumbnailSize, bool show, bool positionable )
{
    addCategory( DB::CategoryPtr( new XMLCategory( text, icon, type, thumbnailSize, show, positionable ) ) );
}

void XMLDB::XMLCategoryCollection::initIdMap()
{
    for( QList<DB::CategoryPtr>::Iterator it = _categories.begin(); it != _categories.end(); ++it )
        static_cast<XMLCategory*>((*it).data())->initIdMap();
}

#include "XMLCategoryCollection.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:

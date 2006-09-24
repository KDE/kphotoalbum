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

#include "MemberMap.h"
#include "DB/Category.h"
#include "MainWindow/DirtyIndicator.h"

using namespace DB;

MemberMap::MemberMap() :QObject(0), _dirty( true ), _loading( false )
{
}

/**
   returns the groups directly available from category (non closure that is)
*/
QStringList MemberMap::groups( const QString& category ) const
{
    return QStringList( _members[ category ].keys() );
}

void MemberMap::deleteGroup( const QString& category, const QString& name )
{
    _members[category].remove(name);
    _dirty = true;
    if ( !_loading )
        MainWindow::DirtyIndicator::markDirty();
}

/**
   return all the members of memberGroup
*/
QStringList MemberMap::members( const QString& category, const QString& memberGroup, bool closure ) const
{
    if ( closure ) {
        if ( _dirty )
            const_cast<MemberMap*>(this)->calculate();
        return _closureMembers[category][memberGroup].toList();
    }
    else
        return _members[category][memberGroup].toList();
}

void MemberMap::setMembers( const QString& category, const QString& memberGroup, const QStringList& members )
{
    _members[category][memberGroup] = members;
    _dirty = true;
    if ( !_loading )
        MainWindow::DirtyIndicator::markDirty();
}

bool MemberMap::isEmpty() const
{
    return _members.empty();
}

/**
   returns true if item is a group for category.
*/
bool MemberMap::isGroup( const QString& category, const QString& item ) const
{
    return _members[category].find(item) != _members[category].end();
}


/**
   return a map from groupName to list of items for category
   example: { USA |-> [Chicago, Grand Canyon, Santa Clara], Denmark |-> [Esbjerg, Odense] }
*/
QMap<QString,StringSet> MemberMap::groupMap( const QString& category ) const
{
    if ( _dirty )
        calculate();

    return _closureMembers[category];
}

/**
   Calculates the closure for group, that is finds all members for group.
   Imagine there is a group called USA, and that this groups has a group inside it called Califonia,
   Califonia consists of members San Fransisco and Los Angeless.
   This function then maps USA to include Califonia, San Fransisco and Los Angeless.
*/
QStringList MemberMap::calculateClosure( QMap<QString,StringSet>& resultSoFar, const QString& category, const QString& group ) const
{
    resultSoFar[group] = StringSet(); // Prevent against cykles.
    StringSet members = _members[category][group];
    StringSet result = members;
    for( StringSet::Iterator it = members.begin(); it != members.end(); ++it ) {
        if ( resultSoFar.contains( *it ) ) {
            result += resultSoFar[*it];
        }
        else if ( isGroup(category, *it ) ) {
            result += calculateClosure( resultSoFar, category, *it );
        }
    }

    resultSoFar[group] = result;
    return result.toList();
}

/**
   This methods create the map _closureMembers from _members
   This is simply to avoid finding the closure each and every time it is needed.
*/
void MemberMap::calculate() const
{
    _closureMembers.clear();
    // run through all categories
    for( QMap< QString,QMap<QString,StringSet> >::ConstIterator categoryIt= _members.begin();
         categoryIt != _members.end(); ++categoryIt ) {

        QString category = categoryIt.key();
        QMap<QString, StringSet> groupMap = categoryIt.data();

        // Run through each of the groups for the given categories
        for( QMapIterator<QString,StringSet> groupIt= groupMap.begin(); groupIt != groupMap.end(); ++groupIt ) {
            QString group = groupIt.key();
            if ( _closureMembers[category].find( group ) == _closureMembers[category].end() ) {
                (void) calculateClosure( _closureMembers[category], category, group );
            }
        }
    }
    _dirty = false;
}

void MemberMap::renameGroup( const QString& category, const QString& oldName, const QString& newName )
{
    _dirty = true;
    if ( !_loading )
        MainWindow::DirtyIndicator::markDirty();
    QMap<QString, StringSet>& groupMap = _members[category];
    groupMap.insert(newName,_members[category][oldName] );
    groupMap.remove( oldName );
    for( QMapIterator<QString,StringSet> it= groupMap.begin(); it != groupMap.end(); ++it ) {
        StringSet& list = it.data();
        if ( list.contains( oldName ) ) {
            list.remove( oldName );
            list.insert( newName );
        }
    }
}

MemberMap::MemberMap( const MemberMap& other )
    : QObject( 0 ), _members( other.memberMap() ), _dirty( true ), _loading( false )
{
}

void MemberMap::deleteItem( DB::Category* category, const QString& name)
{
    _dirty = true;
    if ( !_loading )
        MainWindow::DirtyIndicator::markDirty();
    QMap<QString, StringSet>& groupMap = _members[category->name()];
    for( QMapIterator<QString,StringSet> it= groupMap.begin(); it != groupMap.end(); ++it ) {
        StringSet& items = it.data();
        items.remove( name );
    }
    _members[category->name()].remove(name);
}

void MemberMap::renameItem( DB::Category* category, const QString& oldName, const QString& newName )
{
    _dirty = true;
    if ( !_loading )
        MainWindow::DirtyIndicator::markDirty();
    QMap<QString, StringSet>& groupMap = _members[category->name()];
    for( QMapIterator<QString,StringSet> it= groupMap.begin(); it != groupMap.end(); ++it ) {
        StringSet& items = it.data();
        if (items.contains( oldName ) ) {
            items.remove( oldName );
            items.insert( newName );
        }
    }
    if ( groupMap.contains( oldName ) ) {
        groupMap[newName] = groupMap[oldName];
        groupMap.remove(oldName);
    }
}


MemberMap& MemberMap::operator=( const MemberMap& other )
{
    if ( this != &other ) {
        _members = other.memberMap();
        _dirty = true;
    }
    return *this;
}



void MemberMap::addMemberToGroup( const QString& category, const QString& group, const QString& item )
{
    if ( item.isNull() ) {
        qWarning( "Null item tried inserted into group %s", group.latin1());
        return;
    }


    _members[category][group].insert( item );
    _dirty = true;
    if ( !_loading )
        MainWindow::DirtyIndicator::markDirty();
}

void MemberMap::removeMemberFromGroup( const QString& category, const QString& group, const QString& item )
{
    Q_ASSERT( _members.contains(category) );
    if ( _members[category].contains( group ) )
        _members[category][group].remove( item );
    _dirty = true;
    if ( !_loading )
        MainWindow::DirtyIndicator::markDirty();
}

void MemberMap::addGroup( const QString& category, const QString& group )
{
    if ( ! _members[category].contains( group ) ) {
        _members[category].insert( group, QStringList() );
    }
}

void MemberMap::renameCategory( const QString& oldName, const QString& newName )
{
    _members[newName] = _members[oldName];
    _members.remove(oldName);
}

QMap<QString,QStringList> DB::MemberMap::inverseMap( const QString& category ) const
{
    QMap<QString,QStringList> res;
    const QMap<QString,StringSet>& map =  _members[category];

    for( QMap<QString,StringSet>::ConstIterator mapIt = map.begin(); mapIt != map.end(); ++mapIt ) {
        QString group = mapIt.key();
        StringSet members = mapIt.data();
        for( StringSet::Iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt ) {
            res[*memberIt].append( group );
        }
    }
    return res;
}

void DB::MemberMap::setLoading( bool b )
{
    _loading = b;
}

#include "MemberMap.moc"

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "membermap.h"
#include "options.h"
#include <qtimer.h>

MemberMap::MemberMap() :QObject(0), _dirty( true )
{
    QTimer::singleShot( 0, this, SLOT( init() ) );
}

void MemberMap::init()
{
    connect( Options::instance(), SIGNAL( deletedOption( const QString&, const QString& ) ),
             this, SLOT( deleteOption( const QString&, const QString& ) ) );
    connect( Options::instance(), SIGNAL( renamedOption( const QString&, const QString&, const QString& ) ),
             this, SLOT( renameOption( const QString&, const QString&, const QString& ) ) );
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
}

/**
   return all the members of memberGroup
*/
QStringList MemberMap::members( const QString& category, const QString& memberGroup, bool closure ) const
{
    if ( closure ) {
        if ( _dirty )
            const_cast<MemberMap*>(this)->calculate();
        return _closureMembers[category][memberGroup];
    }
    else
        return _members[category][memberGroup];
}

void MemberMap::setMembers( const QString& category, const QString& memberGroup, const QStringList& members )
{
    _members[category][memberGroup] = members;
    _dirty = true;
}

QDomElement MemberMap::save( QDomDocument doc )
{
    QDomElement top = doc.createElement( QString::fromLatin1( "member-groups" ) );
    for( QMapIterator< QString,QMap<QString,QStringList> > it1= _members.begin(); it1 != _members.end(); ++it1 ) {
        QMap<QString,QStringList> map = it1.data();
        for( QMapIterator<QString,QStringList> it2= map.begin(); it2 != map.end(); ++it2 ) {
            QStringList list = it2.data();
            for( QStringList::Iterator it3 = list.begin(); it3 != list.end(); ++it3 ) {
                QDomElement elm = doc.createElement( QString::fromLatin1( "member" ) );
                top.appendChild( elm );
                elm.setAttribute( QString::fromLatin1( "category" ), it1.key() );
                elm.setAttribute( QString::fromLatin1( "group-name" ), it2.key() );
                elm.setAttribute( QString::fromLatin1( "member" ), *it3 );
            }
        }
    }
    return top;
}

bool MemberMap::isEmpty() const
{
    return _members.empty();
}

void MemberMap::load( const QDomElement& top )
{
    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() ) {
            QDomElement elm = node.toElement();
            QString category = elm.attribute( QString::fromLatin1( "category" ) );
            if ( category.isNull() )
                category = elm.attribute( QString::fromLatin1( "option-group" ) ); // compatible with KimDaBa 2.0
            QString group = elm.attribute( QString::fromLatin1( "group-name" ) );
            QString member = elm.attribute( QString::fromLatin1( "member" ) );
            _members[category][group].append( member );
        }
    }
    _dirty = true;
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
QMap<QString,QStringList> MemberMap::groupMap( const QString& category )
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
QStringList MemberMap::calculateClosure( QMap<QString,QStringList>& resultSoFar, const QString& category, const QString& group )
{
    resultSoFar[group] = QStringList(); // Prevent against cykles.
    QStringList members = _members[category][group];
    QStringList result = members;
    for( QStringList::Iterator it = members.begin(); it != members.end(); ++it ) {
        if ( resultSoFar.contains( *it ) ) {
            result += resultSoFar[*it];
        }
        else if ( isGroup(category, *it ) ) {
            result += calculateClosure( resultSoFar, category, *it );
        }
    }

    QStringList uniq;
    for( QStringList::Iterator it = result.begin(); it != result.end(); ++it ) {
        if ( !uniq.contains(*it) )
            uniq << *it;
    }

    resultSoFar[group] = uniq;
    return uniq;
}

/**
   This methods create the map _closureMembers from _members
   This is simply to avoid finding the closure each and every time it is needed.
*/
void MemberMap::calculate()
{
    _closureMembers.clear();
    // run through all categories
    for( QMapIterator< QString,QMap<QString,QStringList> > categoryIt= _members.begin(); categoryIt != _members.end(); ++categoryIt ) {
        QString category = categoryIt.key();
        QMap<QString, QStringList> groupMap = categoryIt.data();

        // Run through each of the groups for the given categories
        for( QMapIterator<QString,QStringList> groupIt= groupMap.begin(); groupIt != groupMap.end(); ++groupIt ) {
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
    QMap<QString, QStringList>& groupMap = _members[category];
    groupMap.insert(newName,_members[category][oldName] );
    groupMap.remove( oldName );
    for( QMapIterator<QString,QStringList> it= groupMap.begin(); it != groupMap.end(); ++it ) {
        QStringList& list = it.data();
        if ( list.contains( oldName ) ) {
            list.remove( oldName );
            list.append( newName );
        }
    }
    Options::instance()->renameOption( category, oldName, newName );
}

MemberMap::MemberMap( const MemberMap& other )
    : QObject( 0 ), _members( other._members ), _dirty( true )
{
}

void MemberMap::deleteOption( const QString& category, const QString& name)
{
    _dirty = true;
    QMap<QString, QStringList>& groupMap = _members[category];
    for( QMapIterator<QString,QStringList> it= groupMap.begin(); it != groupMap.end(); ++it ) {
        QStringList& list = it.data();
        list.remove( name );
    }
}

void MemberMap::renameOption( const QString& category, const QString& oldName, const QString& newName )
{
    _dirty = true;
    QMap<QString, QStringList>& groupMap = _members[category];
    for( QMapIterator<QString,QStringList> it= groupMap.begin(); it != groupMap.end(); ++it ) {
        QStringList& list = it.data();
        if (list.contains( oldName ) ) {
            list.remove( oldName );
            list.append( newName );
        }
    }
}


MemberMap& MemberMap::operator=( const MemberMap& other )
{
    if ( this != &other ) {
        _members = other._members;
        _dirty = true;
    }
    return *this;
}



void MemberMap::addMemberToGroup( const QString& category, const QString& group, const QString& item )
{
    Q_ASSERT( _members.contains(category) );
    Q_ASSERT( _members[category].contains(group) );
    _members[category][group].append( item );
    _dirty = true;
}

void MemberMap::removeMemberFromGroup( const QString& category, const QString& group, const QString& item )
{
    Q_ASSERT( _members.contains(category) );
    Q_ASSERT( _members[category].contains( group ) );
    _members[category][group].remove( item );
    _dirty = true;
}

void MemberMap::addGroup( const QString& category, const QString& group )
{
    Q_ASSERT( _members.contains(category) );
    if ( ! _members[category].contains( group ) ) {
        _members[category].insert( group, QStringList() );
    }
}

#include "membermap.moc"

/* Copyright (C) 2003-2004 Jesper K. Pedersen <blackie@kde.org>

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
   returns the groups directly available from optionGroup (non closure that is)
*/
QStringList MemberMap::groups( const QString& optionGroup ) const
{
    return QStringList( _members[ optionGroup ].keys() );
}

void MemberMap::deleteGroup( const QString& optionGroup, const QString& name )
{
    _members[optionGroup].remove(name);
    _dirty = true;
}

/**
   return all the members of memberGroup
*/
QStringList MemberMap::members( const QString& optionGroup, const QString& memberGroup, bool closure ) const
{
    if ( closure ) {
        if ( _dirty )
            const_cast<MemberMap*>(this)->calculate();
        return _closureMembers[optionGroup][memberGroup];
    }
    else
        return _members[optionGroup][memberGroup];
}

void MemberMap::setMembers( const QString& optionGroup, const QString& memberGroup, const QStringList& members )
{
    _members[optionGroup][memberGroup] = members;
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
                elm.setAttribute( QString::fromLatin1( "option-group" ), it1.key() );
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
            QString optionGroup = elm.attribute( QString::fromLatin1( "option-group" ) );
            QString group = elm.attribute( QString::fromLatin1( "group-name" ) );
            QString member = elm.attribute( QString::fromLatin1( "member" ) );
            _members[optionGroup][group].append( member );
        }
    }
    _dirty = true;
}

/**
   returns true if item is a group for optionGroup.
*/
bool MemberMap::isGroup( const QString& optionGroup, const QString& item ) const
{
    return _members[optionGroup].find(item) != _members[optionGroup].end();
}


/**
   return a map from groupName to list of items for optionGroup
   example: { USA |-> [Chicago, Grand Canyon, Santa Clara], Denmark |-> [Esbjerg, Odense] }
*/
QMap<QString,QStringList> MemberMap::groupMap( const QString& optionGroup )
{
    if ( _dirty )
        calculate();

    return _closureMembers[optionGroup];
}

/**
   Calculates the closure for group, that is finds all members for group.
   Imagine there is a group called USA, and that this groups has a group inside it called Califonia,
   Califonia consists of members San Fransisco and Los Angeless.
   This function then maps USA to include Califonia, San Fransisco and Los Angeless.
*/
QStringList MemberMap::calculateClosure( QMap<QString,QStringList>& resultSoFar, const QString& optionGroup, const QString& group )
{
    resultSoFar[group] = QStringList(); // Prevent against cykles.
    QStringList members = _members[optionGroup][group];
    QStringList result = members;
    for( QStringList::Iterator it = members.begin(); it != members.end(); ++it ) {
        if ( resultSoFar.contains( *it ) ) {
            result += resultSoFar[*it];
        }
        else if ( isGroup(optionGroup, *it ) ) {
            result += calculateClosure( resultSoFar, optionGroup, *it );
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
    // run through all option groups
    for( QMapIterator< QString,QMap<QString,QStringList> > optionGroupIt= _members.begin(); optionGroupIt != _members.end(); ++optionGroupIt ) {
        QString optionGroup = optionGroupIt.key();
        QMap<QString, QStringList> groupMap = optionGroupIt.data();

        // Run through each of the groups for the given option group
        for( QMapIterator<QString,QStringList> groupIt= groupMap.begin(); groupIt != groupMap.end(); ++groupIt ) {
            QString group = groupIt.key();
            if ( _closureMembers[optionGroup].find( group ) == _closureMembers[optionGroup].end() ) {
                (void) calculateClosure( _closureMembers[optionGroup], optionGroup, group );
            }
        }
    }
    _dirty = false;
}

void MemberMap::renameGroup( const QString& optionGroup, const QString& oldName, const QString& newName )
{
    _dirty = true;
    QMap<QString, QStringList>& groupMap = _members[optionGroup];
    groupMap.insert(newName,_members[optionGroup][oldName] );
    groupMap.remove( oldName );
    for( QMapIterator<QString,QStringList> it= groupMap.begin(); it != groupMap.end(); ++it ) {
        QStringList& list = it.data();
        if ( list.contains( oldName ) ) {
            list.remove( oldName );
            list.append( newName );
        }
    }
    Options::instance()->renameOption( optionGroup, oldName, newName );
}

MemberMap::MemberMap( const MemberMap& other )
    : QObject( 0 ), _members( other._members ), _dirty( true )
{
}

void MemberMap::deleteOption( const QString& optionGroup, const QString& name)
{
    _dirty = true;
    QMap<QString, QStringList>& groupMap = _members[optionGroup];
    for( QMapIterator<QString,QStringList> it= groupMap.begin(); it != groupMap.end(); ++it ) {
        QStringList& list = it.data();
        list.remove( name );
    }
}

void MemberMap::renameOption( const QString& optionGroup, const QString& oldName, const QString& newName )
{
    _dirty = true;
    QMap<QString, QStringList>& groupMap = _members[optionGroup];
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



#include "membermap.moc"

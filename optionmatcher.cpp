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

#include "optionmatcher.h"
#include "options.h"
#include "imageinfo.h"
#include "membermap.h"
#include "imagedb.h"

OptionValueMatcher::OptionValueMatcher( const QString& category, const QString& option )
    :_category( category ), _option( option )
{
}

bool OptionValueMatcher::eval( ImageInfo* info )
{
    info->setMatched( _category, _option );
    if ( info->hasOption( _category, _option ) ) {
        return true;
    }

    QStringList list = ImageDB::instance()->memberMap().members( _category, _option, true );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( info->hasOption( _category, *it ) )
            return true;
    }

    return false;
}



OptionEmptyMatcher::OptionEmptyMatcher( const QString& category )
    :_category( category )
{
}

bool OptionEmptyMatcher::eval( ImageInfo* info )
{
    return info->allMatched( _category );
}



void OptionContainerMatcher::addElement( OptionMatcher* element )
{
    _elements.append( element );
}

bool OptionAndMatcher::eval( ImageInfo* info )
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if ( !(*it)->eval( info ) )
            return false;
    }
    return true;
}



bool OptionOrMatcher::eval( ImageInfo* info )
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if ( (*it)->eval( info ) )
            return true;
    }
    return false;
}



OptionNotMatcher::OptionNotMatcher( OptionMatcher* element )
    :_element( element )
{
}

bool OptionNotMatcher::eval( ImageInfo* info )
{
    return !_element->eval( info );
}

OptionMatcher* OptionValueMatcher::optimize()
{
    return this;
}

OptionMatcher* OptionEmptyMatcher::optimize()
{
    return this;
}

OptionMatcher* OptionContainerMatcher::optimize()
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ) {
        QValueList<OptionMatcher*>::Iterator matcher = it;
        ++it;

        (*matcher) = (*matcher)->optimize();
        if ( *matcher == 0 )
            _elements.remove( matcher );
    }

    if ( _elements.count() == 0 ) {
        delete this;
        return 0;
    }
    else if ( _elements.count() == 1 ) {
        OptionMatcher* res = _elements[0]->optimize();
        _elements.clear();
        delete this;
        return res;
    }

    else
        return this;

}

OptionMatcher* OptionNotMatcher::optimize()
{
    _element = _element->optimize();
    if ( _element == 0 ) {
        delete this;
        return 0;
    }
    return this;
}

OptionContainerMatcher::~OptionContainerMatcher()
{
    for( uint i = 0; i < _elements.count(); ++i )
        delete _elements[i];
}

void OptionValueMatcher::debug(int level) const
{
    qDebug("%s%s: %s", spaces(level).latin1(), _category.latin1(), _option.latin1());
}

void OptionEmptyMatcher::debug( int level ) const
{
    qDebug("%s%s:EMPTY", spaces(level).latin1(), _category.latin1() );
}

void OptionAndMatcher::debug( int level ) const
{
    qDebug("%sAND:", spaces(level).latin1() );
    OptionContainerMatcher::debug( level + 1 );
}

void OptionNotMatcher::debug( int level ) const
{
    qDebug("%sNOT:", spaces(level).latin1() );
    _element->debug( level + 1 );
}

void OptionOrMatcher::debug( int level ) const
{
    qDebug("%sOR:", spaces(level).latin1() );
    OptionContainerMatcher::debug( level + 1 );
}

void OptionContainerMatcher::debug( int level ) const
{
    for( QValueList<OptionMatcher*>::ConstIterator it = _elements.begin(); it != _elements.end(); ++it ) {
        (*it)->debug( level );
    }
}

QString OptionMatcher::spaces(int level ) const
{
    return QString::fromLatin1("").rightJustify(level*3 );
}


OptionMatcher* OptionValueMatcher::clone()
{
    return new OptionValueMatcher( _category, _option );
}

OptionMatcher* OptionEmptyMatcher::clone()
{
    return new OptionEmptyMatcher( _category );
}

OptionMatcher* OptionAndMatcher::clone()
{
    OptionAndMatcher* matcher = new OptionAndMatcher;
    OptionContainerMatcher::clone( matcher );
    return matcher;
}

OptionMatcher* OptionOrMatcher::clone()
{
    OptionOrMatcher* matcher = new OptionOrMatcher;
    OptionContainerMatcher::clone( matcher );
    return matcher;
}

OptionMatcher* OptionNotMatcher::clone()
{
    return new OptionNotMatcher( _element->clone() );
}

void OptionContainerMatcher::clone( OptionContainerMatcher* newMatcher )
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        newMatcher->addElement( (*it)->clone() );
    }
}

OptionMatcher* OptionAndMatcher::optimize()
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ) {
        QValueList<OptionMatcher*>::Iterator  elm = it;
        ++it;
        OptionAndMatcher* child = dynamic_cast<OptionAndMatcher*>(*elm);
        if ( child ) {
            for( QValueList<OptionMatcher*>::Iterator itChild = child->_elements.begin(); itChild != child->_elements.end(); ++itChild ) {
                _elements.prepend( *itChild );
            }
            _elements.remove( elm );
        }
    }
    return OptionContainerMatcher::optimize();
}

OptionMatcher* OptionOrMatcher::optimize()
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ) {
        QValueList<OptionMatcher*>::Iterator  elm = it;
        ++it;
        OptionOrMatcher* child = dynamic_cast<OptionOrMatcher*>(*elm);
        if ( child ) {
            for( QValueList<OptionMatcher*>::Iterator itChild = child->_elements.begin(); itChild != child->_elements.end(); ++itChild ) {
                _elements.prepend( *itChild );
            }
            _elements.remove( elm );
        }
    }
    return OptionContainerMatcher::optimize();
}

OptionMatcher* OptionValueMatcher::normalize()
{
    return clone();
}

OptionMatcher* OptionEmptyMatcher::normalize()
{
    return clone();
}

OptionMatcher* OptionAndMatcher::normalize()
{
    if ( _elements.count() == 0 )
        return clone();
    else if ( _elements.count() == 1 )
        return _elements[0]->normalize();

    OptionMatcher* result = _elements[0]->normalize();
    result = result->optimize();

    QValueList<OptionMatcher*>::Iterator it = _elements.begin();
    ++it;

    for( ; it != _elements.end(); ) {
        OptionMatcher* elm = (*it)->normalize()->optimize();
        ++it;

        OptionMatcher* tmp = normalizeTwo( result, elm );
        delete result;
        delete elm;
        result = tmp;
    }
    return result;
}

OptionMatcher* OptionOrMatcher::normalize()
{
    for( QValueList<OptionMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        OptionMatcher* item = *it;
        (*it) = item->normalize();
        delete item;
    }
    return this;
}

OptionMatcher* OptionNotMatcher::normalize()
{
    return clone();
}

OptionMatcher* OptionAndMatcher::normalizeTwo( OptionMatcher* a, OptionMatcher* b)
{
    // a and b are normalized and optimized.
    QValueList<OptionMatcher*> listA;
    QValueList<OptionMatcher*> listB;

    if ( a->isSimple() )
        listA.append( a );
    else {
        OptionOrMatcher* matcher = dynamic_cast<OptionOrMatcher*>( a );
        if ( matcher )
            listA = matcher->_elements;
        else
            listA.append( a );
    }

    if ( b->isSimple() )
        listB.append( b );
    else {
        OptionOrMatcher* matcher = dynamic_cast<OptionOrMatcher*>( b );
        if ( matcher )
            listB = matcher->_elements;
        else
            listB.append( b );
    }

    OptionOrMatcher* result = new OptionOrMatcher;
    for( QValueList<OptionMatcher*>::Iterator itA = listA.begin(); itA != listA.end(); ++itA ) {
        for( QValueList<OptionMatcher*>::Iterator itB = listB.begin(); itB != listB.end(); ++itB ) {
            OptionAndMatcher* andMatcher = new OptionAndMatcher;
            andMatcher->addElement( (*itA)->clone() );
            andMatcher->addElement( (*itB)->clone() );
            result->addElement( andMatcher );
        }
    }

    return result;
}

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

    QStringList list = Options::instance()->memberMap().members( _category, _option, true );
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

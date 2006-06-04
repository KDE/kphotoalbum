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

#include "CategoryMatcher.h"
#include "Settings/SettingsData.h"
#include "DB/ImageInfo.h"
#include "DB/MemberMap.h"
#include "DB/ImageDB.h"

using namespace DB;

OptionValueMatcher::OptionValueMatcher( const QString& category, const QString& value, bool sign )
{
    _category = category ;
    _option = value;
    _sign = sign;
}

bool OptionValueMatcher::eval( ImageInfoPtr info )
{
    info->setMatched( _category, _option );
    if ( info->hasOption( _category, _option ) ) {
        return _sign;
    }

    QStringList list = DB::ImageDB::instance()->memberMap().members( _category, _option, true );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( info->hasOption( _category, *it ) )
            return _sign;
    }

    return !_sign;
}



OptionEmptyMatcher::OptionEmptyMatcher( const QString& category, bool sign )
{
    _category = category;
    _sign = sign;
}

bool OptionEmptyMatcher::eval( ImageInfoPtr info )
{
    return _sign ? info->allMatched( _category ) : !info->allMatched( _category );
}



void OptionContainerMatcher::addElement( CategoryMatcher* element )
{
    _elements.append( element );
}

bool OptionAndMatcher::eval( ImageInfoPtr info )
{
    for( QValueList<CategoryMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if ( !(*it)->eval( info ) )
            return false;
    }
    return true;
}



bool OptionOrMatcher::eval( ImageInfoPtr info )
{
    for( QValueList<CategoryMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if ( (*it)->eval( info ) )
            return true;
    }
    return false;
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

void OptionOrMatcher::debug( int level ) const
{
    qDebug("%sOR:", spaces(level).latin1() );
    OptionContainerMatcher::debug( level + 1 );
}

void OptionContainerMatcher::debug( int level ) const
{
    for( QValueList<CategoryMatcher*>::ConstIterator it = _elements.begin(); it != _elements.end(); ++it ) {
        (*it)->debug( level );
    }
}

QString CategoryMatcher::spaces(int level ) const
{
    return QString::fromLatin1("").rightJustify(level*3 );
}

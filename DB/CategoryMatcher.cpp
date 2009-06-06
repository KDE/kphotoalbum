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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "CategoryMatcher.h"
#include "DB/ImageInfo.h"
#include "DB/MemberMap.h"
#include "DB/ImageDB.h"
//Added by qt3to4:
 #include <QList>

using namespace DB;

OptionValueMatcher::OptionValueMatcher( const QString& category, const QString& value, bool sign )
{
    _category = category ;
    _option = value;
    _sign = sign;
}

bool OptionValueMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
    // Following block does same as the old statement:
    // info->setMatched(_category, _option)
    alreadyMatched[_category].insert(_option);
    const MemberMap& map = DB::ImageDB::instance()->memberMap();
    const QStringList members = map.members(_category, _option, true);
    alreadyMatched[_category].unite(members.toSet());

    if ( info->hasCategoryInfo( _category, _option ) ) {
        return _sign;
    }

    QStringList list = DB::ImageDB::instance()->memberMap().members( _category, _option, true );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( info->hasCategoryInfo( _category, *it ) )
            return _sign;
    }

    return !_sign;
}



OptionEmptyMatcher::OptionEmptyMatcher( const QString& category, bool sign )
{
    _category = category;
    _sign = sign;
}

bool OptionEmptyMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
    bool allMatched = true;
    Q_FOREACH(const QString& item, info->itemsOfCategory(_category))
    {
        if (!alreadyMatched[_category].contains(item))
        {
            allMatched = false;
            break;
        }
    }
    return _sign ? allMatched : !allMatched;
}



void OptionContainerMatcher::addElement( CategoryMatcher* element )
{
    _elements.append( element );
}

bool OptionAndMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
     for( QList<CategoryMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if (!(*it)->eval(info, alreadyMatched))
            return false;
    }
    return true;
}



bool OptionOrMatcher::eval(ImageInfoPtr info, QMap<QString, StringSet>& alreadyMatched)
{
     for( QList<CategoryMatcher*>::Iterator it = _elements.begin(); it != _elements.end(); ++it ) {
        if ((*it)->eval(info, alreadyMatched))
            return true;
    }
    return false;
}



OptionContainerMatcher::~OptionContainerMatcher()
{
    for( int i = 0; i < _elements.count(); ++i )
        delete _elements[i];
}

void OptionValueMatcher::debug(int level) const
{
    qDebug("%s%s: %s", qPrintable(spaces(level)), qPrintable(_category), qPrintable(_option));
}

void OptionEmptyMatcher::debug( int level ) const
{
    qDebug("%s%s:EMPTY", qPrintable(spaces(level)), qPrintable(_category) );
}

void OptionAndMatcher::debug( int level ) const
{
    qDebug("%sAND:", qPrintable(spaces(level)) );
    OptionContainerMatcher::debug( level + 1 );
}

void OptionOrMatcher::debug( int level ) const
{
    qDebug("%sOR:", qPrintable(spaces(level)) );
    OptionContainerMatcher::debug( level + 1 );
}

void OptionContainerMatcher::debug( int level ) const
{
     for( QList<CategoryMatcher*>::ConstIterator it = _elements.begin(); it != _elements.end(); ++it ) {
        (*it)->debug( level );
    }
}

QString CategoryMatcher::spaces(int level ) const
{
    return QString::fromLatin1("").rightJustified(level*3 );
}

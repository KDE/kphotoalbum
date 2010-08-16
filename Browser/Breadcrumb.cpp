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

#include "Breadcrumb.h"
#include <klocale.h>

int Browser::Breadcrumb::_count = 0;

Browser::Breadcrumb::Breadcrumb(const QString& text, bool isBeginning )
    : _index( ++_count ),_isBeginning( isBeginning), _isView(false), _text( text )
{
}

Browser::Breadcrumb Browser::Breadcrumb::empty()
{
    return Breadcrumb( QString() );
}

Browser::Breadcrumb Browser::Breadcrumb::home()
{
    return Breadcrumb( i18n("All"), true );
}

QString Browser::Breadcrumb::text() const
{
    return _text;
}

bool Browser::Breadcrumb::isBeginning() const
{
    return _isBeginning;
}

bool Browser::Breadcrumb::operator==( const Breadcrumb& other ) const
{
    return other._index == _index;
}

bool Browser::Breadcrumb::operator!=( const Breadcrumb& other ) const
{
    return !(other == *this );
}

Browser::Breadcrumb Browser::Breadcrumb::view()
{
    Breadcrumb res( QString(), false );
    res._isView = true;
    return res;
}

bool Browser::Breadcrumb::isView() const
{
    return _isView;
}

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

#include "folder.h"
#include <klocale.h>
#include "options.h"
#include "imagedb.h"

int Folder::_idCount = 0;

Folder::Folder( const ImageSearchInfo& info, Browser* parent )
    : _index(_idCount++), _browser( parent ), _info( info ), _enabled( true )
{
}


FolderAction::FolderAction(const ImageSearchInfo& info, Browser* browser )
    :  _browser( browser ), _info( info )
{
}

QString FolderAction::path() const
{
    return _info.toString();
}

int Folder::compare( Folder* other, int col, bool asc ) const
{
    Folder* o = static_cast<Folder*>( other );
    if ( !_browser->allowSort() ) {
        Folder* o = static_cast<Folder*>( other );
        if ( _index < o->_index )
            return (asc ? -1 : 1);
        else if ( _index > o->_index )
            return (asc ? 1 : -1);
        else
            return 0;
    }

    else if ( col == 0 )
        return text().compare( o->text() );

    else if ( col == 1 ) {
        if ( _count < o->_count )
            return -1;
        else
            return ( _count != o->_count);
    }
    else
        Q_ASSERT( false );
    return 0;
}

bool FolderAction::allowSort() const
{
    return true;
}

QString FolderAction::title() const
{
    return QString::fromLatin1( "" );
}

QString FolderAction::category() const
{
    return QString::null;
}

void Folder::setEnabled( bool b )
{
    _enabled = b;
}

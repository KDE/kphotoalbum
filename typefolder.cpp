/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "typefolder.h"
#include "options.h"
#include "imagedb.h"
#include "contentfolder.h"
#include <klocale.h>

TypeFolder::TypeFolder( const QString& optionGroup, const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _optionGroup ( optionGroup )
{
    setText( 0, Options::instance()->textForOptionGroup( optionGroup ) );
    QMap<QString, int> map = ImageDB::instance()->classify( _info, _optionGroup );
    int count = map.size();
    setCount( count );

    setText( 1, i18n("1 category", "%n categories", count ) );
    setPixmap( 0, Options::instance()->iconForOptionGroup( _optionGroup ) );
}

FolderAction* TypeFolder::action( bool /* ctrlDown */ )
{
    return new TypeFolderAction( _optionGroup, _info, _browser );
}

TypeFolderAction::TypeFolderAction( const QString& optionGroup, const ImageSearchInfo& info,
                                    Browser* browser )
    :FolderAction( info, browser ), _optionGroup( optionGroup )
{
}

void TypeFolderAction::action()
{
    _browser->clear();

    QMap<QString, int> map = ImageDB::instance()->classify( _info, _optionGroup );
    for( QMapIterator<QString,int> it= map.begin(); it != map.end(); ++it ) {
        if ( it.key() != i18n( "**NONE**" ) ) {
            new ContentFolder( _optionGroup, it.key(), it.data(), _info, _browser );
        }
    }

    // Add the none option to the end
    int i = map[i18n("**NONE**")];
    if ( i != 0 )
        new ContentFolder( _optionGroup, i18n( "**NONE**" ), i, _info, _browser );
}

QString TypeFolderAction::title() const
{
    return Options::instance()->textForOptionGroup( _optionGroup );
}


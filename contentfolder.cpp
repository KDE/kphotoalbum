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

#include "contentfolder.h"
#include "options.h"
#include "typefolder.h"
#include "imagefolder.h"
#include <klocale.h>
#include "imagedb.h"
ContentFolder::ContentFolder( const QString& optionGroup, const QString& value, int count,
                              const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _optionGroup( optionGroup ), _value( value )
{
    if ( value == QString::fromLatin1( "__NONE__" ) )
        setText( i18n( "None (%1)" ).arg(count) );
    else {
        setText( QString::fromLatin1( "%1 (%2)" ).arg( value ).arg( count ) );
        setPixmap( Options::instance()->iconForOptionGroup( optionGroup ) );
    }
}

void ContentFolderAction::action()
{
    ImageSearchInfo info = _info;
    if ( !_optionGroup.isNull() ) {
        // It will be null for the initial element ceated from the browser.
        info.addAnd( _optionGroup, _value );
    }
    _browser->clear();

    QStringList grps = Options::instance()->optionGroups();

    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        new TypeFolder( *it, info, _browser );
    }

    // Image Folders
    int count = ImageDB::instance()->count( info );
    int maxPerPage = Options::instance()->maxImages();

    if ( count < maxPerPage ) {
        new ImageFolder( info, _browser );
    }
    else {
        int last = 1;
        while ( last < count ) {
            new ImageFolder( info, last, QMIN( count, last+maxPerPage-1 ), _browser );
            last += maxPerPage;
        }
    }
}

FolderAction* ContentFolder::action( bool ctrlDown )
{
    if ( ctrlDown ) {
        ImageSearchInfo info = _info;
        info.addAnd( _optionGroup, _value );
        if ( ImageDB::instance()->count( info ) < Options::instance()->maxImages() )
            return new ImageFolderAction( info, -1, -1, _browser );
    }

    return new ContentFolderAction( _optionGroup, _value, _info, _browser );
}

ContentFolderAction::ContentFolderAction( const QString& optionGroup, const QString& value,
                                          const ImageSearchInfo& info, Browser* parent )
    :FolderAction( info, parent ), _optionGroup( optionGroup ), _value( value )
{
}


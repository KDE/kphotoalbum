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

#include "imagefolder.h"
#include <klocale.h>
#include "imagedb.h"
#include "imagesearchinfo.h"
#include "options.h"
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kiconloader.h>

ImageFolder::ImageFolder( const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _from(-1), _to(-1)
{
    int count = ImageDB::instance()->count( info );
    setCount( count );
}

ImageFolder::ImageFolder( const ImageSearchInfo& info, int from, int to, Browser* parent )
    :Folder( info,parent), _from( from ), _to( to )
{
    int count = to - from +1;
    setCount( count );
}

QPixmap ImageFolder::pixmap()
{
    return KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kimdaba" ), KIcon::Desktop, 22 );
}

QString ImageFolder::text() const
{
    if ( _from == -1 )
        return i18n( "View Images" );
    else
        return i18n( "View Images (%1-%2)").arg(_from).arg(_to);
}



void ImageFolderAction::action( BrowserItemFactory* )
{
    ImageDB::instance()->search( _info, _from, _to );

    if ( _addExtraToBrowser ) {
        // Add all the following image fractions to the image list, so the user
        // simply can use the forward button to see the following images.
        int count = ImageDB::instance()->count( _info );
        int maxPerPage = Options::instance()->maxImages();

        if ( count > maxPerPage ) {
            int last = _to;
            while ( last < count ) {
                ImageFolderAction* action =
                    new ImageFolderAction( _info, last, QMIN( count, last+maxPerPage-1 ), _browser );

                // We do not want this new action to create extra items as we do here.
                action->_addExtraToBrowser = false;

                _browser->_list.append( action );
                _browser->emitSignals();
                last += maxPerPage;
            }
        }

        // Only add extra items the first time the action is executed.
        _addExtraToBrowser = false;
    }
}

FolderAction* ImageFolder::action( bool /* ctrlDown */ )
{
    return new ImageFolderAction( _info, _from, _to, _browser );
}

ImageFolderAction::ImageFolderAction( const ImageSearchInfo& info, int from, int to,  Browser* browser )
    : FolderAction( info, browser ), _from(from), _to(to), _addExtraToBrowser( true )
{
}


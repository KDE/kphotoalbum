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

#ifndef IMAGEFOLDER_H
#define IMAGEFOLDER_H
#include "Folder.h"
//Added by qt3to4:
#include <QPixmap>

namespace Browser
{

class ImageFolder :public Folder {
public:
    ImageFolder( const DB::ImageSearchInfo& info, BrowserWidget* parent );
    OVERRIDE FolderAction* action( bool ctrlDown = false );
    OVERRIDE QPixmap pixmap();
    OVERRIDE QString text() const;
    OVERRIDE QString imagesLabel() const;
    OVERRIDE QString videosLabel() const;
};

class ImageFolderAction :public FolderAction
{
public:
    ImageFolderAction( const DB::ImageSearchInfo& info, BrowserWidget* browser );
    ImageFolderAction( const QString& context, BrowserWidget* parent );
    OVERRIDE void action( BrowserItemFactory* );
    OVERRIDE bool showsImages() const { return true; }
    OVERRIDE bool contentView() const { return false; }

private:
    bool _addExtraToBrowser;
    QString _context;
};

}

#endif /* IMAGEFOLDER_H */


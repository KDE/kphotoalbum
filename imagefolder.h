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

#ifndef IMAGEFOLDER_H
#define IMAGEFOLDER_H
#include "folder.h"

class ImageFolder :public Folder {
public:
    ImageFolder( const ImageSearchInfo& info, Browser* parent );
    ImageFolder( const ImageSearchInfo& info, int from, int to, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false );
    virtual QPixmap pixmap();
    virtual QString text() const;
    virtual QString countLabel() const;

private:
    int _from, _to;
};

class ImageFolderAction :public FolderAction
{
public:
    ImageFolderAction( const ImageSearchInfo& info, int from, int to, Browser* browser );
    ImageFolderAction( ImageInfo* context, Browser* parent );
    virtual void action( BrowserItemFactory* );
    virtual bool showsImages() const { return true; }
    virtual bool contentView() const { return false; }

private:
    int _from, _to;
    bool _addExtraToBrowser;
    ImageInfo* _context;
};

#endif /* IMAGEFOLDER_H */


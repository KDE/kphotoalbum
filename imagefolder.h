/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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
    virtual void action( BrowserItemFactory* );
    virtual bool showsImages() const { return true; }
    virtual bool contentView() const { return false; }

private:
    int _from, _to;
    bool _addExtraToBrowser;
};

#endif /* IMAGEFOLDER_H */


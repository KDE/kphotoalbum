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

#ifndef FOLDER_H
#define FOLDER_H
#include <qiconview.h>
#include "browser.h"
#include <qstring.h>
#include "imagesearchinfo.h"

class FolderAction;
class BrowserItemFactory;

class Folder {

public:
    Folder( const ImageSearchInfo& info, Browser* browser );
    virtual ~Folder() {};
    virtual FolderAction* action( bool ctrlDown = false ) = 0;
    void setCount( int count ) { _count = count; }
    virtual QPixmap pixmap() = 0;
    virtual QString text() const = 0;
    virtual int count() { return _count; }
    virtual int compare( Folder* other, int col, bool asc ) const;

    friend class TypeFolder;
    friend class ImageFolder;
    int _index;
    static int _idCount;

    Browser* _browser;
    ImageSearchInfo _info;
    int _count;
};

class FolderAction
{
public:
    FolderAction( const ImageSearchInfo& info, Browser* browser );
    virtual ~FolderAction() {}
    virtual void action( BrowserItemFactory* factory ) = 0;
    virtual bool showsImages() = 0;
    QString path() const;
    virtual bool allowSort() const;
    virtual QString title() const;

protected:
    friend class Browser;
    Browser* _browser;
    ImageSearchInfo _info;
};

#endif /* FOLDER_H */


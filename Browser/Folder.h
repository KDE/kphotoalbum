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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef FOLDER_H
#define FOLDER_H
#include <qiconview.h>
#include "BrowserWidget.h"
#include <qstring.h>
#include "DB/ImageSearchInfo.h"
#include "DB/MediaCount.h"

namespace Browser
{

class FolderAction;
class BrowserItemFactory;

class Folder {

public:
    Folder( const DB::ImageSearchInfo& info, BrowserWidget* browser );
    virtual ~Folder() {};
    virtual FolderAction* action( bool ctrlDown = false ) = 0;
    void setCount( DB::MediaCount count ) { _count = count; }
    virtual QPixmap pixmap() = 0;
    virtual QString text() const = 0;
    virtual DB::MediaCount count() { return _count; }
    virtual int compare( Folder* other, int col, bool asc ) const;
    virtual QString imagesLabel() const = 0;
    virtual QString moviesLabel() const = 0;
    void setEnabled( bool );

    friend class TypeFolder;
    friend class ImageFolder;
    int _index;
    static int _idCount;

    BrowserWidget* _browser;
    DB::ImageSearchInfo _info;
    DB::MediaCount _count;
    bool _enabled;
};

class FolderAction
{
public:
    FolderAction( const DB::ImageSearchInfo& info, BrowserWidget* browser );
    virtual ~FolderAction() {}
    virtual void action( BrowserItemFactory* factory ) = 0;
    virtual bool showsImages() const = 0;
    virtual bool contentView() const = 0;
    QString path() const;
    virtual bool allowSort() const;
    virtual QString title() const;
    virtual QString category() const;

protected:
    friend class BrowserWidget;
    BrowserWidget* _browser;
    DB::ImageSearchInfo _info;
};

}

#endif /* FOLDER_H */


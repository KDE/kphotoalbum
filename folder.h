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

class Folder :public QIconViewItem {

public:
    Folder( const ImageSearchInfo& info, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false ) = 0;

protected:
    Browser* _browser;
    ImageSearchInfo _info;
};

class FolderAction
{
public:
    FolderAction( const ImageSearchInfo& info, Browser* browser );
    virtual ~FolderAction() {}
    virtual void action() = 0;
    virtual bool showsImages() = 0;

protected:
    Browser* _browser;
    ImageSearchInfo _info;
};

#endif /* FOLDER_H */


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

#ifndef CONTENTFOLDER_H
#define CONTENTFOLDER_H
#include "folder.h"

class ContentFolder :public Folder {
public:
    ContentFolder( const QString& optionGroup, const QString& value, int count,
                   const ImageSearchInfo& info, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false );
    virtual QPixmap pixmap();
    virtual QString text() const;
    int compare( Folder* other, int col, bool asc ) const;
    virtual QString countLabel() const;

private:
    QString _optionGroup;
    QString _value;
};

class ContentFolderAction :public FolderAction {

public:
    ContentFolderAction( const QString& optionGroup, const QString& value,
                         const ImageSearchInfo& info, Browser* parent );
    virtual void action( BrowserItemFactory* factory );
    virtual bool showsImages() const { return false; }
    virtual bool contentView() const { return false; }
    virtual bool allowSort() const;
    virtual QString title() const;

private:
    QString _optionGroup;
    QString _value;
};

#endif /* CONTENTFOLDER_H */


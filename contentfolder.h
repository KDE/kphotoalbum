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

#ifndef CONTENTFOLDER_H
#define CONTENTFOLDER_H
#include "folder.h"

class ContentFolder :public Folder {
public:
    ContentFolder( const QString& optionGroup, const QString& value, int count,
                   const ImageSearchInfo& info, Browser* parent );
    virtual FolderAction* action( bool ctrlDown = false );

private:
    QString _optionGroup;
    QString _value;
};

class ContentFolderAction :public FolderAction {

public:
    ContentFolderAction( const QString& optionGroup, const QString& value,
                         const ImageSearchInfo& info, Browser* parent );
    virtual void action();
    virtual bool showsImages() { return false; }

private:
    QString _optionGroup;
    QString _value;
};

#endif /* CONTENTFOLDER_H */


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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef BROWSER_TYPEFOLDERACTION_H
#define BROWSER_TYPEFOLDERACTION_H

#include "Folder.h"
#include "DB/Category.h"
#include <qmap.h>

namespace Browser
{
class BrowserItemFactory;
class BrowserItem;

class TypeFolderAction :public FolderAction {

public:
    TypeFolderAction( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* parent  );
    virtual void action( BrowserItemFactory* factory );
    virtual bool showsImages() const { return false; }
    virtual bool contentView() const;
    virtual QString title() const;
    DB::CategoryPtr category() const;
    virtual DB::Category::ViewType viewType() const;

protected:
    bool populateBrowserWithHierachy( DB::CategoryItem* parentCategoryItem, const QMap<QString, uint>& images,
                                      const QMap<QString, uint>& videos, BrowserItemFactory* factory, BrowserItem* parentBrowserItem );
    void populateBrowserWithoutHierachy( const QMap<QString, uint>& images,
                                         const QMap<QString, uint>& videos, BrowserItemFactory* factory );

private:
    const DB::CategoryPtr _category;
};

}


#endif /* BROWSER_TYPEFOLDERACTION_H */


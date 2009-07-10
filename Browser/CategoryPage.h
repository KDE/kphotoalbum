/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef CATEGORYPAGE_H
#define CATEGORYPAGE_H
#include "BrowserPage.h"
#include <DB/Category.h>
#include <DB/ImageSearchInfo.h>
#include "DB/CategoryPtr.h"

class QAbstractItemModel;
class FlatCategoryModel;
class BrowserWidget;

namespace Browser {

/**
 * \brief The Browser page for categories.
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 */
class CategoryPage :public BrowserPage
{
public:
    CategoryPage( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info, BrowserWidget* browser );
    OVERRIDE void activate();
    OVERRIDE BrowserPage* activateChild( const QModelIndex& );
    OVERRIDE DB::Category::ViewType viewType() const;
    OVERRIDE bool isViewChangeable() const;

    DB::CategoryPtr category() const;

private:
    const DB::CategoryPtr _category;
    QAbstractItemModel* _model;
};

}

#endif /* CATEGORYPAGE_H */


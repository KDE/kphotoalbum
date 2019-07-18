/* Copyright (C) 2014 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef TYPES_H
#define TYPES_H

#include <QObject>

namespace RemoteControl
{

class Types
{
    Q_GADGET

public:
    enum Page { Startup,
                UnconnectedPage,
                OverviewPage,
                CategoryItemsPage,
                CategoryListPage,
                ThumbnailsPage,
                ImageViewerPage,
                DiscoverPage };
    enum class ViewType { CategoryItems,
                          Thumbnails,
                          Images };
    enum CategoryViewType { CategoryListView,
                            CategoryIconView };
    Q_ENUMS(Page)
    Q_ENUMS(ViewType)
    Q_ENUMS(CategoryViewType)
};

using Page = Types::Page;
using ViewType = Types::ViewType;
using CategoryViewType = Types::CategoryViewType;

enum class SearchType { Categories,
                        CategoryItems,
                        Images };

using ImageId = int;

const ImageId DISCOVERYID = -1000;

}

#endif // TYPES_H

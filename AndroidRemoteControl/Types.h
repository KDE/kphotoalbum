/* SPDX-FileCopyrightText: 2014 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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

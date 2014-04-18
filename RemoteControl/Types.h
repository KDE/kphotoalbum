#ifndef TYPES_H
#define TYPES_H

#include <QObject>

namespace RemoteControl
{

class Types {
    Q_GADGET

public:
    enum Page {UnconnectedPage, OverviewPage, CategoryItemsPage, ThumbnailsPage, ImageViewerPage};
    enum class ViewType { CategoryItems, Thumbnails, Images };
    Q_ENUMS(Page)
    Q_ENUMS(ViewType)
};

using Page = Types::Page;
using ViewType = Types::ViewType;

enum class SearchType { Categories, CategoryItems, Images };

}

#endif // TYPES_H

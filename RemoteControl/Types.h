#ifndef TYPES_H
#define TYPES_H

#include <QObject>

namespace RemoteControl
{

class Types {
    Q_GADGET

public:
    enum class Page {Unconnected, Overview, CategoryItems, Thumbnails, ImageViewer};
    Q_ENUMS(Page)
};

enum class ViewType { CategoryItems, Thumbnails, Images };
enum class SearchType { Categories, CategoryItems, Images };

}

#endif // TYPES_H

#ifndef THUMBNAILVIEW_ENUMS_H
#define THUMBNAILVIEW_ENUMS_H
#include "DB/ResultId.h"
#include <QSet>

namespace ThumbnailView
{
enum SortDirection { NewestFirst, OldestFirst };
enum Order { ViewOrder, SortedOrder };
enum CoordinateSystem {ViewportCoordinates, ContentsCoordinates };
enum VisibleState { FullyVisible, PartlyVisible };

typedef QSet<DB::ResultId> IdSet;
}

#endif /* THUMBNAILVIEW_ENUMS_H */


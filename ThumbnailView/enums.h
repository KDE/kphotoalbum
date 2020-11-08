/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef THUMBNAILVIEW_ENUMS_H
#define THUMBNAILVIEW_ENUMS_H

namespace ThumbnailView
{
enum SortDirection { NewestFirst,
                     OldestFirst };
enum Order { ViewOrder,
             SortedOrder };
enum CoordinateSystem { ViewportCoordinates,
                        ContentsCoordinates };
enum VisibleState { FullyVisible,
                    PartlyVisible };

enum SelectionUpdateMethod { ClearSelection,
                             MaintainSelection };
/** @short Operation mode of selection in ThumbnailView.
 *
 * The SelectionMode determines how collapsed stacks and partially
 * selected stacks are handled when determining which images to include
 * in the selection.
 */
enum SelectionMode {
    NoExpandCollapsedStacks, //< @short Only include images that have been explicitly marked.
    ExpandCollapsedStacks, //< @short For collapsed stacks, include the whole stack instead of just the stack head.
    IncludeAllStacks //< @short Include the whole stack, even if the stack is expanded and only parts of it are selected.
};

}

#endif /* THUMBNAILVIEW_ENUMS_H */
// vi:expandtab:tabstop=4 shiftwidth=4:

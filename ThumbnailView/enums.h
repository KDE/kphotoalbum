/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

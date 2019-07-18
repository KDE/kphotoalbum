/* Copyright (C) 2003-2011 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef IMAGEMANAGER_ENUMS_H
#define IMAGEMANAGER_ENUMS_H

namespace ImageManager
{
/** @short Priority of an image request
 *
 * The higher the priority, the sooner the image is expected to be decoded
 * */
enum Priority {
    BuildThumbnails, //< @short Requests generated through the "Rebuild Thumbnails" command
    BuildScopeThumbnails, //< @short thumbnails in current search scope to be rebuidl
    ThumbnailInvisible, //< @short Thumbnails in current search scope, but invisible
    ViewerPreload, // < @short Image that will be displayed later
    BatchTask, /**< @short Requests like resizing images for HTML pages
                *
                * As they are requested by user, they are expected to finish
                * sooner than invisible thumbnails */
    ThumbnailVisible, /**< @short Thumbnail visible on screen right now (might get invalidated later) */
    Viewer /**< @short Image is visible in the viewer right now */,
    LastPriority /**< @short Boundary for list of queues */
};

enum ThumbnailBuildStart { StartNow,
                           StartDelayed };

enum StopAction { StopAll,
                  StopOnlyNonPriorityLoads };

}

#endif // IMAGEMANAGER_ENUMS_H
// vi:expandtab:tabstop=4 shiftwidth=4:

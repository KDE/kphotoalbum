/* SPDX-FileCopyrightText: 2003-2011 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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

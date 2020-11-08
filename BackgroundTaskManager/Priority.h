/* SPDX-FileCopyrightText: 2012 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BACKGROUNDTASKAMANAGER_PRIORITY_H
#define BACKGROUNDTASKAMANAGER_PRIORITY_H
namespace BackgroundTaskManager
{
enum Priority {
    ForegroundCycleRequest = 0,
    ForegroundThumbnailRequest = 1,
    BackgroundTask = 2, // This is only a marker between foreground and background, do not use as a priority.
    BackgroundVideoInfoRequest = 2,
    BackgroundVideoThumbnailRequest = 3,
    BackgroundVideoPreviewRequest = 4,
    SIZE_OF_PRIORITY_QUEUE // Must be after the last one, and the last one MUST be the highest.
};

}

#endif // BACKGROUNDTASKAMANAGER_PRIORITY_H
// vi:expandtab:tabstop=4 shiftwidth=4:

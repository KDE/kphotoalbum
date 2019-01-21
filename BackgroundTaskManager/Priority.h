/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

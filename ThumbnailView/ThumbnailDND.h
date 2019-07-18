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
#ifndef THUMBNAILDND_H
#define THUMBNAILDND_H

#include <QObject>

#include "ThumbnailComponent.h"

class QDragEnterEvent;
class QDropEvent;
class QDragLeaveEvent;
class QDragMoveEvent;

namespace ThumbnailView
{

class ThumbnailDND : public QObject, private ThumbnailComponent
{
    Q_OBJECT

public:
    explicit ThumbnailDND(ThumbnailFactory *factory);
    void contentsDragMoveEvent(QDragMoveEvent *event);
    void contentsDragLeaveEvent(QDragLeaveEvent *);
    void contentsDropEvent(QDropEvent *event);
    void contentsDragEnterEvent(QDragEnterEvent *event);

private slots:
    void realDropEvent();

private:
    void removeDropIndications();
};
}

#endif /* THUMBNAILDND_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

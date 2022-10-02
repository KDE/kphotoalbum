// SPDX-FileCopyrightText: 2003-2022 Jesper K. Pedersen <blackie@kde.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef THUMBNAILDND_H
#define THUMBNAILDND_H

#include "ThumbnailComponent.h"

#include <QObject>

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

private Q_SLOTS:
    void realDropEvent();

private:
    void removeDropIndications();
};
}

#endif /* THUMBNAILDND_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

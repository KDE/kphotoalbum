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
#ifndef KEYBOARDEVENTHANDLER_H
#define KEYBOARDEVENTHANDLER_H

#include <QObject>

#include "ThumbnailComponent.h"
#include "enums.h"

class QKeyEvent;
class ThumbnailFactory;

namespace ThumbnailView
{
/**
 * @brief The KeyboardEventHandler class handles keyboard input for the thumbnail widget.
 *
 * Specifically, the following keyboard interactions are handled:
 *  - Setting and unsetting tokens on images (a-z)
 *  - Setting the rating for images (1-5)
 *  - Stopping video thumbnail cycling when Control is pressed
 *  - Showing the Viewer when Enter is pressed.
 */
class KeyboardEventHandler :public QObject, public ThumbnailComponent
{
    Q_OBJECT

public:
    explicit KeyboardEventHandler( ThumbnailFactory* factory );
    bool keyPressEvent( QKeyEvent* event );
    bool keyReleaseEvent( QKeyEvent* );

signals:
    void showSelection();


private:
};
}

#endif /* KEYBOARDEVENTHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2021-2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef KEYBOARDEVENTHANDLER_H
#define KEYBOARDEVENTHANDLER_H

#include "ThumbnailComponent.h"
#include "enums.h"

#include <QObject>

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
 *  - Applying filters for tokens and ratings.
 *  - Clearing the current filter
 */
class KeyboardEventHandler : public QObject, public ThumbnailComponent
{
    Q_OBJECT

public:
    explicit KeyboardEventHandler(ThumbnailFactory *factory);
    bool keyPressEvent(QKeyEvent *event);
    bool keyReleaseEvent(QKeyEvent *);

Q_SIGNALS:
    void showSelection();
    void showSearch();

private:
};
}

#endif /* KEYBOARDEVENTHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

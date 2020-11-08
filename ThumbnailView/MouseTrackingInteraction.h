/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MOUSETRACKINGINTERACTION_H
#define MOUSETRACKINGINTERACTION_H

#include "MouseInteraction.h"
#include "ThumbnailComponent.h"

#include <QMouseEvent>

namespace DB
{
class FileName;
}

namespace ThumbnailView
{

class MouseTrackingInteraction : public QObject, public MouseInteraction, private ThumbnailComponent
{
    Q_OBJECT

public:
    explicit MouseTrackingInteraction(ThumbnailFactory *factory);
    bool mouseMoveEvent(QMouseEvent *) override;

signals:
    void fileIdUnderCursorChanged(const DB::FileName &id);

private:
    void updateStackingIndication(QMouseEvent *event);
    void handleCursorOverNewIcon();

private:
    bool m_cursorWasAtStackIcon;
};
}

#endif /* MOUSETRACKINGINTERACTION_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

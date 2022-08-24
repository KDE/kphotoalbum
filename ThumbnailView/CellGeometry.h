// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2022 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CELLGEOMETRY_H
#define CELLGEOMETRY_H
#include "ThumbnailComponent.h"

#include <QRect>
#include <QSize>

class QPixmap;
class QRect;
class QSize;

namespace DB
{
class Id;
}

namespace ThumbnailView
{
class ThumbnailFactory;

class CellGeometry : public ThumbnailComponent
{
public:
    void flushCache();
    explicit CellGeometry(ThumbnailFactory *factory);
    QSize cellSize() const;
    static QSize preferredIconSize();
    static QSize baseIconSize();
    QRect iconGeometry(const QPixmap &pixmap) const;
    int textHeight() const;
    QRect cellTextGeometry() const;
    void calculateCellSize();

private:
    void calculateTextHeight();
    void calculateCellTextGeometry();

    bool m_cacheInitialized = false;
    int m_textHeight = 0;
    QSize m_cellSize;
    QRect m_cellTextGeometry;
};

}

#endif /* CELLGEOMETRY_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

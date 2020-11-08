/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef THUMBNAILCOMPONENT_H
#define THUMBNAILCOMPONENT_H

namespace DB
{
class Id;
}

namespace ThumbnailView
{
class ThumbnailFactory;
class ThumbnailPainter;
class ThumbnailWidget;
class CellGeometry;
class ThumbnailModel;

class ThumbnailComponent
{
public:
    explicit ThumbnailComponent(ThumbnailFactory *factory);

    ThumbnailModel *model();
    const ThumbnailModel *model() const;

    CellGeometry *cellGeometryInfo();
    const CellGeometry *cellGeometryInfo() const;

    ThumbnailWidget *widget();
    const ThumbnailWidget *widget() const;

private:
    ThumbnailFactory *m_factory;
};

}

#endif /* THUMBNAILCOMPONENT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

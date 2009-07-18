/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef THUMBNAILCOMPONENT_H
#define THUMBNAILCOMPONENT_H

namespace DB {class ResultId; }

namespace ThumbnailView
{
class ThumbnailFactory;
class ThumbnailPainter;
class ThumbnailWidget;
class CellGeometry;
class ThumbnailModel;
class ThumbnailCache;

class ThumbnailComponent
{
public:
    ThumbnailComponent( ThumbnailFactory* factory );

    ThumbnailModel* model();
    const ThumbnailModel* model() const;

    CellGeometry* cellGeometryInfo();
    const CellGeometry* cellGeometryInfo() const;

    ThumbnailWidget* widget();
    const ThumbnailWidget* widget() const;

    ThumbnailPainter* painter();
    const ThumbnailPainter* painter() const;

    ThumbnailCache* cache();
    const ThumbnailCache* cache() const;
private:
    ThumbnailFactory* _factory;
};

}

#endif /* THUMBNAILCOMPONENT_H */


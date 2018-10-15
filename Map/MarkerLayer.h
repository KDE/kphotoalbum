/* Copyright (C) 2018 The KPhotoAlbum Development Team

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

#ifndef MARKERLAYER_H
#define MARKERLAYER_H

// Marble includes
#include <marble/GeoDataCoordinates.h>
#include <marble/LayerInterface.h>

namespace Map
{

class MarkerLayer : public Marble::LayerInterface
{

public:
    explicit MarkerLayer();
    QStringList renderPosition() const;
    bool render( Marble::GeoPainter* painter, Marble::ViewportParams* viewport,
                 const QString& renderPos, Marble::GeoSceneLayer* layer );
    void addMarker( double lon, double lat );

private: // Variables
    QList<Marble::GeoDataCoordinates> m_markers;
};

}

#endif // MARKERLAYER_H

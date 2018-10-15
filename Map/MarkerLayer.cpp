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

// Local includes
#include "MarkerLayer.h"

// Marble includes
#include <marble/GeoPainter.h>

// Qt includes
#include <QDebug>

namespace
{
const QStringList RENDER_POSITION = QStringList( { QStringLiteral( "SURFACE" ) } );
}

Map::MarkerLayer::MarkerLayer()
{
}

QStringList Map::MarkerLayer::renderPosition() const
{
    return RENDER_POSITION;
}

bool Map::MarkerLayer::render( Marble::GeoPainter* painter, Marble::ViewportParams* viewport,
                               const QString& renderPos, Marble::GeoSceneLayer* layer )
{
    Q_UNUSED( viewport );
    Q_UNUSED( renderPos );
    Q_UNUSED( layer );

    painter->setRenderHint( QPainter::Antialiasing, true );
    painter->setPen( QPen( QBrush( QColor::fromRgb( 255, 0, 0, 255 ) ), 3.0, Qt::SolidLine, Qt::RoundCap ) );

    for ( const Marble::GeoDataCoordinates& coordinates : m_markers ) {
        painter->drawEllipse( coordinates, 10, 10 );
    }

    return true;
}

void Map::MarkerLayer::addMarker( double lon, double lat )
{
    m_markers.append( Marble::GeoDataCoordinates( lon, lat ) );
}

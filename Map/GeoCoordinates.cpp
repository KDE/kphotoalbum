/* Copyright (C) 2018 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "GeoCoordinates.h"

bool Map::GeoCoordinates::hasCoordinates() const
{
    return m_hasCoordinates;
}

double Map::GeoCoordinates::lon() const
{
    return m_lon;
}

double Map::GeoCoordinates::lat() const
{
    return m_lat;
}

double Map::GeoCoordinates::alt() const
{
    return m_alt;
}

bool Map::GeoCoordinates::hasAltitude() const
{
    return m_hasAlt;
}

void Map::GeoCoordinates::setLatLon(const double lat, const double lon)
{
    m_lat = lat;
    m_lon = lon;
    m_hasCoordinates = true;
}

void Map::GeoCoordinates::setAlt(const double alt)
{
    m_alt = alt;
    m_hasAlt = true;
}

Map::GeoCoordinates::Pair Map::GeoCoordinates::makePair(const double lat1, const double lon1,
                                                        const double lat2, const double lon2)
{
    Map::GeoCoordinates coordinates1;
    coordinates1.setLatLon(lat1, lon1);
    Map::GeoCoordinates coordinates2;
    coordinates2.setLatLon(lat2, lon2);
    return Pair(coordinates1, coordinates2);
}

Map::GeoCoordinates::operator QString() const
{
    return QStringLiteral("(%1, %2)").arg(m_lon).arg(m_lat);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

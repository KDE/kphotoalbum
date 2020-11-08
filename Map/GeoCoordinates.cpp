/* SPDX-FileCopyrightText: 2018-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

Map::GeoCoordinates::operator QString() const
{
    return QStringLiteral("(%1, %2)").arg(m_lon).arg(m_lat);
}

Map::GeoCoordinates::LatLonBox::LatLonBox(double north, double south, double east, double west)
    : north(north)
    , south(south)
    , east(east)
    , west(west)
{
}

bool Map::GeoCoordinates::LatLonBox::isNull() const
{
    return north == 0 && south == 0 && east == 0 && west == 0;
}
Map::GeoCoordinates::LatLonBox::operator QString() const
{
    return QStringLiteral("(N%1, S%2, E%3, W%4)").arg(north).arg(south).arg(east).arg(west);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

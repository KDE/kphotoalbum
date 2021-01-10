// SPDX-FileCopyrightText: 2018-2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "GeoCoordinates.h"

#include <marble/GeoDataLatLonAltBox.h>

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

Map::GeoCoordinates::LatLonBox::LatLonBox(const Marble::GeoDataLatLonBox &box)
    : north(box.north(Marble::GeoDataCoordinates::Degree))
    , south(box.south(Marble::GeoDataCoordinates::Degree))
    , east(box.east(Marble::GeoDataCoordinates::Degree))
    , west(box.west(Marble::GeoDataCoordinates::Degree))
{
}

bool Map::GeoCoordinates::LatLonBox::isNull() const
{
    return north == 0 && south == 0 && east == 0 && west == 0;
}

bool Map::GeoCoordinates::LatLonBox::contains(const Map::GeoCoordinates &point) const
{
    // increase size by delta in all directions for the check
    // this fixes numerical issues with those images that lie directly on the border
    const double delta = 0.000001;
    const double lat = point.lat();
    if (lat < south - delta || lat > north + delta) {
        return false;
    }
    const double lon = point.lon();
    if (((lon < west - delta || lon > east + delta)
         && (west < east))
        || ((lon < west - delta && lon > east + delta)
            && (east < west))) {
        return false;
    }
    return true;
}
Map::GeoCoordinates::LatLonBox::operator QString() const
{
    return QStringLiteral("(N%1, S%2, E%3, W%4)").arg(north).arg(south).arg(east).arg(west);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

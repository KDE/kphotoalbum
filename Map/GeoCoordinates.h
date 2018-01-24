/* Copyright (C) 2018 Tobias Leupold <tobias.leupold@gmx.de>

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

#ifndef GEOCOORDINATES_H
#define GEOCOORDINATES_H

// Libkgeomap includes
#include <KGeoMap/GeoCoordinates>

namespace Map
{

class GeoCoordinates
{

public:
    bool hasCoordinates();
    qreal lon();
    qreal lat();
    qreal alt();
    bool hasAltitude();
    KGeoMap::GeoCoordinates kgeomapCoordinates() const;
    void setLatLon(qreal lat, qreal lon);
    void setAlt(qreal alt);
    typedef QPair<GeoCoordinates, GeoCoordinates> Pair;
    static Pair makePair(const qreal lat1, const qreal lon1, const qreal lat2, const qreal lon2);

private: // Variables
    KGeoMap::GeoCoordinates m_kgeomapCoordinates;

};

}

Q_DECLARE_METATYPE(Map::GeoCoordinates::Pair)

#endif // GEOCOORDINATES_H

// vi:expandtab:tabstop=4 shiftwidth=4:

/* SPDX-FileCopyrightText: 2018-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GEOCOORDINATES_H
#define GEOCOORDINATES_H

#include <QString>

namespace Map
{

class GeoCoordinates
{

public:
    bool hasCoordinates() const;
    double lon() const;
    double lat() const;
    double alt() const;
    bool hasAltitude() const;
    void setLatLon(const double lat, const double lon);
    void setAlt(const double alt);

    struct LatLonBox {
        LatLonBox() = default;
        LatLonBox(double north, double south, double east, double west);
        bool isNull() const;
        operator QString() const;
        double north = 0;
        double south = 0;
        double east = 0;
        double west = 0;
    };

    operator QString() const;

private: // Variables
    double m_lat;
    double m_lon;
    double m_alt;
    bool m_hasCoordinates = false;
    bool m_hasAlt = false;
};

}

#endif // GEOCOORDINATES_H

// vi:expandtab:tabstop=4 shiftwidth=4:

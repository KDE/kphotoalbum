/* Copyright (C) 2015 Johannes Zarl-Zierl <johannes@zarl.at>

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
#include "SearchMarkerTiler.h"

// libkgeomap includes

// Local includes
#include <Map/MapView.h>

Map::SearchMarkerTiler::SearchMarkerTiler(KGeoMap::ModelHelper *const modelHelper, QObject *const parent)
    : ItemMarkerTiler(modelHelper, parent)
{
    m_mapView = dynamic_cast<MapView *>(parent);
}

Map::SearchMarkerTiler::~SearchMarkerTiler()
{
}

KGeoMap::GroupState Map::SearchMarkerTiler::getGlobalGroupState()
{
    if (m_mapView->regionSelected()) {
        return KGeoMap::ItemMarkerTiler::getGlobalGroupState() | KGeoMap::RegionSelectedAll;
    } else {
        return KGeoMap::ItemMarkerTiler::getGlobalGroupState();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

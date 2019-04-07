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

#ifndef SEARCHMARKERTILER_H
#define SEARCHMARKERTILER_H

// Libkgeomap includes
#include <KGeoMap/ItemMarkerTiler>
#include <KGeoMap/GroupState>

namespace KGeoMap
{
   class ModelHelper;
}

namespace Map
{

class MapView;

/**
 * @brief The SearchMarkerTiler class
 * This behaves exactly like a KGeoMap::ItemMarkerTiler, except
 * that it always reports all objects to be
 * "inside a region of interest on the map".
 *
 * This whole class is basically a workaround to enable the "clear selection"
 * button when the MapView is in search mode.
 */
class SearchMarkerTiler : public KGeoMap::ItemMarkerTiler
{
public:
   SearchMarkerTiler(KGeoMap::ModelHelper *const modelHelper, QObject *const parent=0);
   ~SearchMarkerTiler() override;
   /**
    * @brief getGlobalGroupState
    * @return KGeoMap::ItemMarkerTiler::getGlobalGroupState() | KGeoMapRegionSelectedAll.
    */
   KGeoMap::GroupState getGlobalGroupState() override;

private: // Variables
    MapView* m_mapView;
};

}

#endif // SEARCHMARKERTILER_H

// vi:expandtab:tabstop=4 shiftwidth=4:

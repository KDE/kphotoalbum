/* Copyright (C) 2014 Johannes Zarl <johannes@zarl.at>

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

#ifndef MAPMARKERMODELHELPER_H
#define MAPMARKERMODELHELPER_H

// libkgeomap includes
#include <KGeoMap/ModelHelper>

// Local includes
#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>

// Qt classes
class QAbstractItemModel;
class QItemSelectionModel;
class QModelIndex;
class QPixmap;
class QPoint;
class QSize;
class QStandardItemModel;

// Kde classes
class QUrl;

namespace KGeoMap
{

// libkgeomap classes
class GeoCoordinates;

}

namespace Map
{

class MapMarkerModelHelper : public KGeoMap::ModelHelper
{
    Q_OBJECT

public:
    MapMarkerModelHelper();
    ~MapMarkerModelHelper() override;

    /**
     * This adds an item to the map
     */
    void addImage(const DB::ImageInfo &image);
    void addImage(const DB::ImageInfoPtr image);
    /**
     * This clears the map
     */
    void clearItems();

    // ------------------------------ ModelHelper API
    bool itemCoordinates(const QModelIndex &index,
                         KGeoMap::GeoCoordinates *const coordinates) const override;
    QAbstractItemModel *model() const override;
    QItemSelectionModel *selectionModel() const override;
    KGeoMap::ModelHelper::Flags modelFlags() const override;
    KGeoMap::ModelHelper::Flags itemFlags(const QModelIndex &index) const override;
    bool itemIcon(const QModelIndex &index,
                  QPoint *const offset,
                  QSize *const size,
                  QPixmap *const pixmap,
                  QUrl *const url) const override;

private: // Variables
    QStandardItemModel *m_itemModel;
    QItemSelectionModel *m_itemSelectionModel;

private slots:
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
};

}

#endif // MAPMARKERMODELHELPER_H

// vi:expandtab:tabstop=4 shiftwidth=4:

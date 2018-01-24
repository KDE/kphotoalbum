/* Copyright (C) 2014-2018 Tobias Leupold <tobias.leupold@gmx.de>

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

#ifndef MAPVIEW_H
#define MAPVIEW_H

// Qt includes
#include <QWidget>

// Local includes
#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include "GeoCoordinates.h"

// Qt classes
class QLabel;
class QPushButton;

namespace KGeoMap
{
class MapWidget;
class ItemMarkerTiler;
}

namespace Map
{

// Local classes
class MapMarkerModelHelper;

class MapView : public QWidget
{
    Q_OBJECT

public:
    /**
     * UsageType: determines whether the widget is used as a standalone widget
     * or within another widget (e.g. the AnnotationDialog).
     * @see Viewer::ViewerWidget::UsageType
     */
    enum UsageType {
        InlineMapView,
        MapViewWindow
    };

    /**
     * MapStatus: determines the visibility and text of the status label and the visibility of the
     * map, depending on the availability of coordinates of the image(s) that are displayed.
     */
    enum MapStatus {
        Loading,
        ImageHasCoordinates,
        ImageHasNoCoordinates,
        NoImagesHaveNoCoordinates,
        SomeImagesHaveNoCoordinates,
        SearchCoordinates
    };

    explicit MapView(QWidget *parent = 0, UsageType type = InlineMapView);
    ~MapView() override;

    /**
     * Removes all images from the map.
     */
    void clear();

    /**
     * Add an image to the map.
     */
    void addImage(const DB::ImageInfo &image);
    void addImage(const DB::ImageInfoPtr image);

    /**
     * Sets the map's zoom so that all images on the map are visible.
     * If no images have been added, the zoom is not altered.
     */
    void zoomToMarkers();

    /**
     * Sets the state of the "Show Thumbnails" button on the map's control widget.
     */
    void setShowThumbnails(bool state);

    /**
     * This sets the status label text and it's visibility, as well as the visibilty of the map
     * itself to the state indicated by the given MapStatus.
     */
    void displayStatus(MapStatus status);

    GeoCoordinates::Pair getRegionSelection() const;
    bool regionSelected() const;

Q_SIGNALS:
    void signalRegionSelectionChanged();
    void displayStatusChanged(MapStatus);

public slots:
    /**
     * Centers the map on the coordinates of the given image.
     */
    void setCenter(const DB::ImageInfo &image);
    void setCenter(const DB::ImageInfoPtr image);

private slots:
    void saveSettings();
    void setLastCenter();

private: // Variables
    KGeoMap::MapWidget *m_mapWidget;
    KGeoMap::ItemMarkerTiler *m_itemMarkerTiler;
    MapMarkerModelHelper *m_modelHelper;
    QLabel *m_statusLabel;
    QPushButton *m_setLastCenterButton;
    GeoCoordinates m_lastCenter;
};

}

#endif // MAPVIEW_H

// vi:expandtab:tabstop=4 shiftwidth=4:

/* Copyright (C) 2014-2019 The KPhotoAlbum Development Team

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

#ifndef MAPVIEW_H
#define MAPVIEW_H

// Local includes
#include "config-kpa-marble.h"
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
#include "GeoCoordinates.h"

// Marble includes
#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLatLonBox.h>
#include <marble/LayerInterface.h>

// Qt includes
#include <QList>
#include <QPixmap>
#include <QWidget>

// Marble classes
namespace Marble
{
class MarbleWidget;
}
// Local includes
#include "GeoCoordinates.h"
#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>

// Qt classes
class QLabel;
class QPushButton;

namespace Map
{

/**
 * UsageType: determines whether the widget is used as a standalone widget
 * or within another widget (e.g. the AnnotationDialog).
 * @see Viewer::ViewerWidget::UsageType
 */
enum class UsageType {
    InlineMapView,
    MapViewWindow
};

/**
 * MapStatus: determines the visibility and text of the status label and the visibility of the
 * map, depending on the availability of coordinates of the image(s) that are displayed.
 */
enum class MapStatus {
    Loading,
    ImageHasCoordinates,
    ImageHasNoCoordinates,
    NoImagesHaveNoCoordinates,
    SomeImagesHaveNoCoordinates,
    SearchCoordinates
};

class MapView
    : public QWidget,
      public Marble::LayerInterface
{
    Q_OBJECT

public:
    explicit MapView(QWidget *parent = nullptr, UsageType type = UsageType::InlineMapView);
    ~MapView() override = default;

    /**
     * Removes all images from the map.
     */
    void clear();

    /**
     * Add an image to the map.
     */
    void addImage(DB::ImageInfoPtr image);

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

    // LayerInterface:
    /**
     * @brief renderPosition tells the LayerManager what layers we (currently) want to paint on.
     * Part of the LayerInterface; called by the LayerManager.
     * @return
     */
    QStringList renderPosition() const override;
    /**
     * @brief Render all markers onto the marbleWidget.
     * Part of the LayerInterface; called by the LayerManager.
     * @param painter the painter used by the LayerManager
     * @param viewport
     * @param renderPos the layer name
     * @param layer always \c nullptr
     * @return \c true (return value is discarded by LayerManager::renderLayers())
     */
    bool render(Marble::GeoPainter *painter, Marble::ViewportParams *,
                const QString &renderPos, Marble::GeoSceneLayer *) override;

Q_SIGNALS:
    void signalRegionSelectionChanged();
    void displayStatusChanged(MapStatus);

public slots:
    /**
     * Centers the map on the coordinates of the given image.
     */
    void setCenter(const DB::ImageInfoPtr image);

private slots:
    void saveSettings();
    void setLastCenter();
    void updateRegionSelection(const Marble::GeoDataLatLonBox &selection);
#ifndef MARBLE_HAS_regionSelected_NEW
    // remove once we don't care about Marble v17.12.3 and older anymore
    void updateRegionSelectionOld(const QList<double> &selection);
#endif

private: // Variables
    Marble::MarbleWidget *m_mapWidget;
    QLabel *m_statusLabel;
    QPushButton *m_setLastCenterButton;
    GeoCoordinates m_lastCenter;
    QWidget *m_kpaButtons;
    QWidget *m_floaters;

    // FIXME(jzarl): dirty hack to get it working
    // if this should work efficiently with a large number of images,
    // some spatially aware data structure probably needs to be used
    // (e.g. binning images by location)
    QList<DB::ImageInfoPtr> m_images;

    Marble::GeoDataLatLonBox m_markersBox;
    bool m_showThumbnails;
    QPixmap m_pin;
    Marble::GeoDataLatLonBox m_regionSelection;
    bool m_regionSelected = false;
};

}

#endif // MAPVIEW_H

// vi:expandtab:tabstop=4 shiftwidth=4:

// SPDX-FileCopyrightText: 2016-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include "GeoCoordinates.h"
#include "enums.h"

#include <DB/ImageInfo.h>
#include <DB/ImageInfoPtr.h>
#include <kpabase/config-kpa-marble.h>

#include <QList>
#include <QPixmap>
#include <QWidget>
#include <marble/GeoDataLatLonAltBox.h>
#include <marble/LayerInterface.h>

namespace DB
{
class ImageSearchInfo;
}
namespace Marble
{
class MarbleWidget;
}

class QLabel;
class QPushButton;

namespace Map
{
class GeoBin;
class GeoCluster;

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

/**
 * \brief A conveniently 64bit word-sized type that holds an imprecise representation of a GeoCoordinate.
 * 64 bit allow for enough precision to represent the lat/lon part of the GeoCoordinate of a GeoBin,
 * while also having nice properties for being used as a key to a hashmap lookup.
 */
using GeoBinAddress = quint64;

class MapView
    : public QWidget,
      public Marble::LayerInterface
{
    Q_OBJECT

public:
    explicit MapView(QWidget *parent, UsageType type);
    ~MapView() override = default;

    /**
     * Removes all images from the map.
     */
    void clear();

    /**
     * Add an image to the map.
     * If you fill the Map using this method, don't forget to call buildImageClusters() afterwards.
     * @return \c true, if the image has coordinates and was added, \c false otherwise.
     */
    bool addImage(DB::ImageInfoPtr image);

    /**
     * @brief addImages adds images matching a search info to the map.
     * @param searchInfo
     */
    void addImages(const DB::ImageSearchInfo &searchInfo);

    /**
     * @brief buildImageClusters creates the GeoClusters that are used to group images that are close to each other.
     * This function is automatically called by addImages(),
     * but you need to call it yourself when adding individual images using addImage().
     */
    void buildImageClusters();
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
     * @brief Sets the state of the "Group Thumbnails" button on lthe map's control widget.
     */
    void setGroupThumbnails(bool state);

    /**
     * @brief Returns the appropriate map style depending on the state of the "Show Thumbnails" and "Group Thumbnails" buttons.
     * @return the currently active MapStyle
     */
    MapStyle mapStyle() const;

    /**
     * This sets the status label text and it's visibility, as well as the visibilty of the map
     * itself to the state indicated by the given MapStatus.
     */
    void displayStatus(MapStatus status);

    GeoCoordinates::LatLonBox getRegionSelection() const;
    bool regionSelected() const;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

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
     * @param viewPortParams information about the region being in view
     * @param renderPos the layer name
     * @return \c true (return value is discarded by LayerManager::renderLayers())
     */
    bool render(Marble::GeoPainter *painter, Marble::ViewportParams *viewPortParams,
                const QString &renderPos, Marble::GeoSceneLayer * /*nullptr*/) override;

    /**
     * @return The size of the rendered thumbnails/markers in pixels.
     */
    int markerSize() const;
    /**
     * @brief Set the markerSize for rendering on the map
     * Note: Apart from thumbnails, this also affects the size of clusters and therefore the clustering.
     * @param markerSizePx a size in pixels
     */
    void setMarkerSize(int markerSizePx);

Q_SIGNALS:
    void newRegionSelected(Map::GeoCoordinates::LatLonBox coordinates);
    void displayStatusChanged(MapStatus);

public Q_SLOTS:
    /**
     * Centers the map on the coordinates of the given image.
     */
    void setCenter(const DB::ImageInfoPtr image);

    void increaseMarkerSize();
    void decreaseMarkerSize();

private Q_SLOTS:
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
    /**
     * If the user clicks a cluster on the map, it gets preselected (and highlighted) before the mouse release event finalizes the selection.
     * @see mousePressEvent
     * @see mouseReleaseEvent
     * @see render
     */
    const GeoCluster *m_preselectedCluster = nullptr;

    // filled by addImage()
    QHash<GeoBinAddress, GeoBin *> m_baseBins;
    QList<GeoCluster *> m_geoClusters;

    Marble::GeoDataLatLonBox m_markersBox;
    bool m_showThumbnails = true;
    bool m_groupThumbnails = true;
    QPixmap m_pin;
    Marble::GeoDataLatLonBox m_regionSelection;
    bool m_regionSelected = false;
    int m_markerSize;
};

}

#endif // MAPVIEW_H

// vi:expandtab:tabstop=4 shiftwidth=4:

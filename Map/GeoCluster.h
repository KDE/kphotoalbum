// SPDX-FileCopyrightText: 2019-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef MAP_GEOCLUSTER_H
#define MAP_GEOCLUSTER_H

#include <DB/ImageInfoPtr.h>

#include <marble/GeoDataCoordinates.h>
#include <marble/GeoDataLatLonAltBox.h>

namespace Marble
{
class GeoDataLatLonBox;
class GeoPainter;
class ViewportParams;
}

namespace ImageManager
{
class ThumbnailCache;
}

namespace Map
{

class GeoCoordinates;

enum class MapStyle {
    ShowPins,
    ShowThumbnails
};

struct ThumbnailParams {
    const QPixmap &alternatePixmap;
    const ImageManager::ThumbnailCache *cache;
};

class GeoCluster
{
public:
    explicit GeoCluster(int lvl);
    virtual ~GeoCluster() = default;

    void addSubCluster(const GeoCluster *subCluster);
    /**
     * @brief boundingRegion computes the bounding region for the GeoCluster
     * All images in the GeoCluster are within the boundingRegion.
     * The result is only computed once at the first call to the method.
     * @return a GeoDataLatLonBox containing all images in all sub-clusters.
     */
    virtual Marble::GeoDataLatLonAltBox boundingRegion() const;
    /**
     * @brief center
     * @return the center of the boundingRegion
     */
    virtual Marble::GeoDataCoordinates center() const;

    /**
     * @brief regionForPoint checks whether the given screen coordinates match the GeoCluster.
     * The corresponding bounding box is computed the same way as in the render method,
     * matching against the GeoClusters own bounding box or against its sub-clusters as appropriate.
     * @param pos
     * @param viewPortParams
     * @return The matching GeoCluster if the position matches, or a \c nullptr otherwise.
     */
    virtual const GeoCluster *regionForPoint(QPoint pos, const Marble::ViewportParams &viewPortParams) const;

    void render(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const ThumbnailParams &thumbs, MapStyle style) const;
    /**
     * @brief size
     * The result is only computed once at the first call to the method.
     * @return the number of images in all sub-clusters
     */
    virtual int size() const;

    /**
     * @brief isEmpty
     * @return \c true, if size is 0, \c false otherwise.
     */
    bool isEmpty() const;

private:
    mutable int m_size = 0;
    mutable bool m_subItemsView = false;
    QList<const GeoCluster *> m_subClusters = {};

protected:
    mutable Marble::GeoDataLatLonAltBox m_boundingRegion;
    const int m_level;
    /**
     * @brief renderSubItems renders the sub-items of this GeoCluster.
     * @param painter
     * @param viewPortParams
     * @param thumbs handle for the thumbnail cache and an alternate pixmap
     * @param style
     */
    virtual void renderSubItems(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const ThumbnailParams &thumbs, MapStyle style) const;
};

/**
 * @brief The GeoBin class holds a number of images that are grouped into the same bin.
 * I.e. they are in the direct vicinity of each other.
 */
class GeoBin : public GeoCluster
{
public:
    GeoBin();
    void addImage(DB::ImageInfoPtr image);
    Marble::GeoDataLatLonAltBox boundingRegion() const override;
    int size() const override;

private:
    mutable QHash<DB::ImageInfoPtr, QPixmap> m_scaledThumbnailCache; ///< local cache for scaled thumbnails
    QList<DB::ImageInfoPtr> m_images;

protected:
    void renderSubItems(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const ThumbnailParams &thumbs, MapStyle style) const override;
};

/**
 * @brief extendGeoDataLatLonBox extend the given GeoDataLatLonBox to encompass the given coordinates.
 * @param box
 * @param coords
 */
void extendGeoDataLatLonBox(Marble::GeoDataLatLonBox &box, const Map::GeoCoordinates &coords);
} //namespace

#endif

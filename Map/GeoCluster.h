/* Copyright (C) 2019 The KPhotoAlbum Development Team

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

namespace Map
{

class GeoCoordinates;

enum class MapStyle {
    ShowPins,
    ShowThumbnails
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
    void render(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const QPixmap &alternatePixmap, MapStyle style) const;
    /**
     * @brief size
     * The result is only computed once at the first call to the method.
     * @return the number of images in all sub-clusters
     */
    virtual int size() const;

private:
    mutable int m_size = 0;
    QList<const GeoCluster *> m_subClusters;

protected:
    mutable Marble::GeoDataLatLonAltBox m_boundingRegion;
    const int m_level;
    /**
     * @brief renderSubItems renders the sub-items of this GeoCluster.
     * @param painter
     * @param viewPortParams
     * @param alternatePixmap
     * @param style
     */
    virtual void renderSubItems(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const QPixmap &alternatePixmap, MapStyle style) const;
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
    QList<DB::ImageInfoPtr> m_images;

protected:
    void renderSubItems(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const QPixmap &alternatePixmap, MapStyle style) const override;
};

/**
 * @brief extendGeoDataLatLonBox extend the given GeoDataLatLonBox to encompass the given coordinates.
 * @param box
 * @param coords
 */
void extendGeoDataLatLonBox(Marble::GeoDataLatLonBox &box, const Map::GeoCoordinates &coords);
} //namespace

#endif

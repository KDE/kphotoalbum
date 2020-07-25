/* Copyright (C) 2019-2020 The KPhotoAlbum Development Team

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

#include "GeoCluster.h"
#include "GeoCoordinates.h"
#include "Logging.h"

#include <DB/ImageInfo.h>
#include <ImageManager/ThumbnailCache.h>

#include <KLocalizedString>
#include <QPen>
#include <QSize>
#include <marble/GeoDataLatLonBox.h>
#include <marble/GeoPainter.h>
#include <marble/ViewportParams.h>

namespace
{
// when the angular resolution is smaller than fineResolution, all details should be shown
constexpr qreal FINE_RESOLUTION = 0.000001;
// size of the markers in screen coordinates (pixel)
constexpr int MARKER_SIZE_PX = 40;
// the scale factor of the bounding box compared to the bounding box as drawn on the map
constexpr qreal BOUNDING_BOX_SCALEFACTOR = 1.2;

/**
 * @brief screenSize computes the screen size of a geographical region in pixels.
 * If one of the bounding box edges is not visible, a null QizeF is retunred.
 * @param viewPortParams the parameters of the current view port
 * @param box the geographical region
 * @return the size in pixels, or a null size
 */
QSizeF screenSize(const Marble::ViewportParams &viewPortParams, const Marble::GeoDataLatLonBox &box, bool debug = false)
{
    qreal east;
    qreal north;
    qreal west;
    qreal south;
    // if a point is not visible on screen, screenCoordinates() returns false
    // the result is still usable, though
    bool onScreen;
    onScreen = viewPortParams.screenCoordinates(box.east(Marble::GeoDataCoordinates::Radian),
                                                box.north(Marble::GeoDataCoordinates::Radian),
                                                east, north);
    onScreen &= viewPortParams.screenCoordinates(box.west(Marble::GeoDataCoordinates::Radian),
                                                 box.south(Marble::GeoDataCoordinates::Radian),
                                                 west, south);
    if (debug) {
        qCDebug(MapLog) << "coordinates" << east << "-" << west << "," << north << "-" << south << "are" << (onScreen ? "on screen" : "not (fully) on screen");
    }
    return QSizeF { qAbs(east - west), qAbs(north - south) };
}

/**
 * @brief screenRegion translates a bounding region into screen coordinates for drawing it.
 * The region has the size of what is actually drawn, not the size of the bounding region itself.
 * @param viewPortParams
 * @param region
 * @return
 */
QRectF screenRegion(const Marble::ViewportParams &viewPortParams, const Marble::GeoDataLatLonBox region)
{
    const QSizeF areaSizePx = screenSize(viewPortParams, region);
    // drawing a larger area gets nicer results on average:
    const qreal heightPx = qMax(BOUNDING_BOX_SCALEFACTOR * areaSizePx.height(), (qreal)MARKER_SIZE_PX);
    const qreal widthPx = qMax(BOUNDING_BOX_SCALEFACTOR * areaSizePx.width(), (qreal)MARKER_SIZE_PX);
    qreal left;
    qreal top;
    viewPortParams.screenCoordinates(region.west(Marble::GeoDataCoordinates::Radian),
                                     region.north(Marble::GeoDataCoordinates::Radian),
                                     left, top);
    return QRectF(left, top, widthPx, heightPx);
}
} //namespace

Marble::GeoDataLatLonAltBox Map::GeoCluster::boundingRegion() const
{
    if (m_boundingRegion.isEmpty()) {
        for (const auto &subCluster : m_subClusters) {
            m_boundingRegion |= subCluster->boundingRegion();
        }
    }
    return m_boundingRegion;
}

Marble::GeoDataCoordinates Map::GeoCluster::center() const
{
    return boundingRegion().center();
}

Marble::GeoDataLatLonBox Map::GeoCluster::regionForPoint(QPoint pos, const Marble::ViewportParams &viewPortParams) const
{
    const QRectF screenRect = screenRegion(viewPortParams, boundingRegion());
    if (!screenRect.contains(pos))
        return {};

    if (m_subItemsView) {
        qCDebug(MapLog) << "GeoCluster matches point, but delegating to subClusters first.";
        for (const auto &subCluster : m_subClusters) {
            const Marble::GeoDataLatLonBox box = subCluster->regionForPoint(pos, viewPortParams);
            if (!box.isEmpty())
                return box;
        }
    }
    qCDebug(MapLog) << "GeoCluster containing" << size() << "images matches point.";
    return boundingRegion();
}

void Map::GeoCluster::render(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const ThumbnailParams &thumbs, Map::MapStyle style) const
{
    if (viewPortParams.resolves(boundingRegion(), 2 * MARKER_SIZE_PX) || size() == 1
        || (viewPortParams.angularResolution() < FINE_RESOLUTION)) {
        m_subItemsView = true;
        // if the region takes up enough screen space, we should display the subclusters individually.
        // if all images have the same coordinates (null bounding region), this will never happen
        // -> in this case, show the images when we're zoomed in enough
        renderSubItems(painter, viewPortParams, thumbs, style);
    } else {
        m_subItemsView = false;
        qCDebug(MapLog) << "GeoCluster has" << size() << "images.";
        painter->setOpacity(0.5);
        const QRectF screenRect = screenRegion(viewPortParams, boundingRegion());
        painter->drawRect(center(), screenRect.width(), screenRect.height());
        painter->setOpacity(1);
        QPen pen = painter->pen();
        painter->setPen(QPen(Qt::black));
        painter->drawText(center(), i18nc("The number of images in an area of the map", "%1", size()), -0.5 * MARKER_SIZE_PX, 0.5 * MARKER_SIZE_PX, MARKER_SIZE_PX, MARKER_SIZE_PX, QTextOption(Qt::AlignCenter));
        painter->setPen(pen);
    }
}

int Map::GeoCluster::size() const
{
    if (m_size == 0) {
        for (const auto &subCluster : m_subClusters) {
            m_size += subCluster->size();
        }
    }
    return m_size;
}

void Map::GeoCluster::renderSubItems(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const ThumbnailParams &thumbs, Map::MapStyle style) const
{
    for (const auto &subCluster : m_subClusters) {
        subCluster->render(painter, viewPortParams, thumbs, style);
    }
}

Map::GeoCluster::GeoCluster(int lvl)
    : m_level(lvl)
{
}

void Map::GeoCluster::addSubCluster(const Map::GeoCluster *subCluster)
{
    m_subClusters.append(subCluster);
}

Map::GeoBin::GeoBin()
    : GeoCluster(0)
{
}

void Map::GeoBin::addImage(DB::ImageInfoPtr image)
{
    m_images.append(image);
    extendGeoDataLatLonBox(m_boundingRegion, image->coordinates());
}

Marble::GeoDataLatLonAltBox Map::GeoBin::boundingRegion() const
{
    return m_boundingRegion;
}

int Map::GeoBin::size() const
{
    return m_images.size();
}

void Map::GeoBin::renderSubItems(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const ThumbnailParams &thumbs, Map::MapStyle style) const
{
    const auto viewPort = viewPortParams.viewLatLonAltBox();
    qCDebug(MapLog) << "GeoBin: drawing individual images";
    for (const DB::ImageInfoPtr &image : m_images) {
        const Marble::GeoDataCoordinates pos(image->coordinates().lon(), image->coordinates().lat(),
                                             image->coordinates().alt(),
                                             Marble::GeoDataCoordinates::Degree);
        if (viewPort.contains(pos)) {
            if (style == MapStyle::ShowPins) {
                painter->drawPixmap(pos, thumbs.alternatePixmap);
            } else {
                // FIXME(l3u) Maybe we should cache the scaled thumbnails?
                painter->drawPixmap(pos, thumbs.cache->lookup(image->fileName()).scaled(QSize(MARKER_SIZE_PX, MARKER_SIZE_PX), Qt::KeepAspectRatio));
            }
        }
    }
}

void Map::extendGeoDataLatLonBox(Marble::GeoDataLatLonBox &box, const Map::GeoCoordinates &coords)
{
    Marble::GeoDataLatLonBox addition { coords.lat(), coords.lat(), coords.lon(), coords.lon(), Marble::GeoDataCoordinates::Degree };
    // let GeoDataLatLonBox::united() take care of the edge cases
    box |= addition;
}

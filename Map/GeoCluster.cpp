// SPDX-FileCopyrightText: 2019-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
 * @param minSizePx
 * @return the region in screen coordinates
 */
QRectF screenRegion(const Marble::ViewportParams &viewPortParams, const Marble::GeoDataLatLonBox region, int minSizePx)
{
    const QSizeF areaSizePx = screenSize(viewPortParams, region);
    // drawing a larger area gets nicer results on average:
    const qreal heightPx = qMax(BOUNDING_BOX_SCALEFACTOR * areaSizePx.height(), (qreal)minSizePx);
    const qreal widthPx = qMax(BOUNDING_BOX_SCALEFACTOR * areaSizePx.width(), (qreal)minSizePx);
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

const Map::GeoCluster *Map::GeoCluster::regionForPoint(QPoint pos, const Marble::ViewportParams &viewPortParams) const
{
    // only check child items if the cluster was not drawn on the map:
    if (m_renderedRegion.isEmpty()) {
        for (const auto &subCluster : m_subClusters) {
            auto cluster = subCluster->regionForPoint(pos, viewPortParams);
            if (cluster && !cluster->isEmpty())
                return cluster;
        }
    } else if (m_renderedRegion.contains(pos)) {
        qCDebug(MapLog) << "GeoCluster containing" << size() << "images matches point.";
        return this;
    }
    return nullptr;
}

void Map::GeoCluster::render(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const ThumbnailParams &thumbs, Map::MapStyle style) const
{
    if (style == MapStyle::ForceShowThumbnails || size() == 1
        || viewPortParams.resolves(boundingRegion(), 2 * thumbs.thumbnailSizePx)
        || (viewPortParams.angularResolution() < FINE_RESOLUTION)) {
        m_renderedRegion = {};
        // if the region takes up enough screen space, we should display the subclusters individually.
        // if all images have the same coordinates (null bounding region), this will never happen
        // -> in this case, show the images when we're zoomed in enough
        renderSubItems(painter, viewPortParams, thumbs, style);
    } else {
        qCDebug(MapLog) << "GeoCluster has" << size() << "images.";
        QPen pen = painter->pen();
        const auto opacity = painter->opacity();
        painter->setOpacity(0.5);
        const QRectF screenRect = screenRegion(viewPortParams, boundingRegion(), thumbs.thumbnailSizePx);
        painter->drawRect(center(), screenRect.width(), screenRect.height());
        m_renderedRegion = painter->regionFromRect(center(), screenRect.width(), screenRect.height());
        painter->setOpacity(opacity);
        painter->setPen(QPen(Qt::black));
        painter->drawText(center(), i18nc("The number of images in an area of the map", "%1", size()), -0.5 * thumbs.thumbnailSizePx, 0.5 * thumbs.thumbnailSizePx, thumbs.thumbnailSizePx, thumbs.thumbnailSizePx, QTextOption(Qt::AlignCenter));
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

bool Map::GeoCluster::isEmpty() const
{
    return (size() == 0);
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
    qCDebug(MapLog) << "GeoBin: drawing" << m_images.count() << "individual images";
    for (const DB::ImageInfoPtr &image : m_images) {
        const Marble::GeoDataCoordinates pos(image->coordinates().lon(), image->coordinates().lat(),
                                             image->coordinates().alt(),
                                             Marble::GeoDataCoordinates::Degree);
        if (viewPort.contains(pos)) {
            if (style == MapStyle::ShowPins) {
                painter->drawPixmap(pos, thumbs.alternatePixmap);
            } else {
                if (thumbs.thumbnailSizePx != m_thumbnailSizePx) {
                    m_scaledThumbnailCache.clear();
                    m_thumbnailSizePx = thumbs.thumbnailSizePx;
                }
                if (!m_scaledThumbnailCache.contains(image)) {
                    QPixmap thumb = thumbs.cache->lookup(image->fileName()).scaled(QSize(thumbs.thumbnailSizePx, thumbs.thumbnailSizePx), Qt::KeepAspectRatio);
                    m_scaledThumbnailCache.insert(image, thumb);
                }
                painter->drawPixmap(pos, m_scaledThumbnailCache.value(image));
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

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

// Local includes
#include "MapView.h"
#include "Logging.h"

#include <DB/ImageDB.h>
#include <DB/ImageSearchInfo.h>
#include <ImageManager/ThumbnailCache.h>
#include <MainWindow/Logging.h>

#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <QAction>
#include <QDebug>
#include <QElapsedTimer>
#include <QLabel>
#include <QLoggingCategory>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <marble/GeoDataLatLonAltBox.h>
#include <marble/GeoPainter.h>
#include <marble/MarbleWidget.h>
#include <marble/RenderPlugin.h>
#include <marble/ViewportParams.h>

namespace
{
const QString MAPVIEW_FLOATER_VISIBLE_CONFIG_PREFIX = QStringLiteral("MarbleFloaterVisible ");
const QStringList MAPVIEW_RENDER_POSITION({ QStringLiteral("HOVERS_ABOVE_SURFACE") });
const QVector<QString> WANTED_FLOATERS { QStringLiteral("Compass"),
                                         QStringLiteral("Scale Bar"),
                                         QStringLiteral("Navigation"),
                                         QStringLiteral("Overview Map") };

// when the angular resolution is smaller than fineResolution, all details should be shown
constexpr qreal FINE_RESOLUTION = 0.000001;
// size of the markers in screen coordinates (pixel)
constexpr int MARKER_SIZE_PX = 40;
// levels of clustering for geo coordinates
constexpr int MAP_CLUSTER_LEVELS = 10;
static_assert(MAP_CLUSTER_LEVELS > 0, "At least one level of clustering is needed for the map.");
static_assert(MAP_CLUSTER_LEVELS < 32, "See coarsenBinAddress to know why this is a bad idea.");

/**
 * @brief computeBinAddress calculates a "bin" for grouping coordinates that are near each other.
 * Using a signed 32-bit integer value allows for 2 decimal places of either coordinate part,
 * which is roughly equivalent to a spatial resolution of 1 km.
 * @param coords (more or less) precise coordinates
 * @return imprecise coordinates
 */
Map::GeoBinAddress computeBinAddress(const Map::GeoCoordinates &coords)
{
    qint64 lat = qRound(coords.lat() * 100);
    qint64 lon = qRound(coords.lon() * 100);
    return static_cast<quint64>((lat << 32) + lon);
}

/**
 * @brief coarsenBinAddress takes A GeoBinAddress and reduces its precision.
 * @param addr the address to be reduced in accuracy
 * @param level how many binary digits should be nulled out
 * @return
 */
Map::GeoBinAddress coarsenBinAddress(Map::GeoBinAddress addr, int level)
{
    // zero out the rightmost bits
    quint64 mask = 0xffffffffffffffff << level;
    // duplicate the mask onto the higher 32 bits
    mask = ((mask << 32) & mask);
    // apply mask
    return addr & mask;
}

/**
 * @brief buildClusterMap fills the lodMap by putting the GeoBin in GeoClusters
 * of decreasing levels of detail.
 * @param lodMap a vector containing a <GeoBinAddress, GeoCluster*> map for each level of detail
 * @param binAddress the GeoBinAddress for the newly added GeoBin
 * @param bin the GeoBin to add to the lodMap
 */
void buildClusterMap(QVector<QHash<Map::GeoBinAddress, Map::GeoCluster *>> &lodMap,
                     Map::GeoBinAddress binAddress, const Map::GeoBin *bin)
{
    const Map::GeoCluster *cluster = bin;
    for (int lvl = 1; lvl <= MAP_CLUSTER_LEVELS; lvl++) {
        QHash<Map::GeoBinAddress, Map::GeoCluster *> &map = lodMap[lvl - 1];
        binAddress = coarsenBinAddress(binAddress, lvl);
        qCDebug(MapLog) << "adding GeoCluster with address" << binAddress << "at level" << lvl;
        if (map.contains(binAddress)) {
            map[binAddress]->addSubCluster(cluster);
            break;
        } else {
            map.insert(binAddress, new Map::GeoCluster(lvl));
            map[binAddress]->addSubCluster(cluster);
            cluster = map[binAddress];
        }
    }
}

/**
 * @brief extendGeoDataLatLonBox extend the given GeoDataLatLonBox to encompass the given coordinates.
 * @param box
 * @param coords
 */
void extendGeoDataLatLonBox(Marble::GeoDataLatLonBox &box, const Map::GeoCoordinates &coords)
{
    if (box.isEmpty()) {
        box.setEast(coords.lon(), Marble::GeoDataCoordinates::Degree);
        box.setWest(coords.lon(), Marble::GeoDataCoordinates::Degree);
        box.setNorth(coords.lat(), Marble::GeoDataCoordinates::Degree);
        box.setSouth(coords.lat(), Marble::GeoDataCoordinates::Degree);
    } else {
        if (box.east(Marble::GeoDataCoordinates::Degree) < coords.lon()) {
            box.setEast(coords.lon(), Marble::GeoDataCoordinates::Degree);
        }
        if (box.west(Marble::GeoDataCoordinates::Degree) > coords.lon()) {
            box.setWest(coords.lon(), Marble::GeoDataCoordinates::Degree);
        }
        if (box.north(Marble::GeoDataCoordinates::Degree) < coords.lat()) {
            box.setNorth(coords.lat(), Marble::GeoDataCoordinates::Degree);
        }
        if (box.south(Marble::GeoDataCoordinates::Degree) > coords.lat()) {
            box.setSouth(coords.lat(), Marble::GeoDataCoordinates::Degree);
        }
    }
}
}

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
    // TODO(jzarl): check how this compares to e.g. the center of all coordinates instead:
    return boundingRegion().center();
}

void Map::GeoCluster::render(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const QPixmap &alternatePixmap, Map::MapStyle style) const
{
    const auto viewPort = viewPortParams.viewLatLonAltBox();

    if (viewPortParams.resolves(boundingRegion(), MARKER_SIZE_PX)
        || (boundingRegion().isNull() && viewPortParams.angularResolution() < m_resolution)) {
        // if the region takes up enough screen space, we should display the subclusters individually.
        // if all images have the same coordinates (null bounding region), this will never happen
        // -> in this case, show the images when we're zoomed in enough
        for (const auto &subCluster : m_subClusters) {
            subCluster->render(painter, viewPortParams, alternatePixmap, style);
        }
    } else {
        qCDebug(MapLog) << "GeoCluster at" << center().toString() << "has" << size() << "images.";
        painter->setOpacity(0.5);
        if (viewPortParams.angularResolution() < m_resolution) {
            qCDebug(MapLog) << "GeoCluster: drawing area";
            // high resolution -> draw in geo coordinates to represent the area of the images
            // the size was empirically determined
            qreal sizeDeg = 1.5 * (boundingRegion().width(Marble::GeoDataCoordinates::Degree) + boundingRegion().height(Marble::GeoDataCoordinates::Degree));
            // sometimes, the boundingRegion is much smaller than the 40px circle
            sizeDeg = qMax(sizeDeg, m_resolution);
            // true -> size is in degree, not screen coordinates
            painter->drawEllipse(center(), sizeDeg, sizeDeg, true);
        } else {
            qCDebug(MapLog) << "GeoCluster: drawing marker";
            // low resolution -> draw in screen coordinates to keep the region visible
            painter->drawEllipse(center(), MARKER_SIZE_PX, MARKER_SIZE_PX);
        }
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

Map::GeoCluster::GeoCluster(int lvl)
    : m_resolution(FINE_RESOLUTION * (2 << lvl))
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

void Map::GeoBin::render(Marble::GeoPainter *painter, const Marble::ViewportParams &viewPortParams, const QPixmap &alternatePixmap, MapStyle style) const
{
    const auto viewPort = viewPortParams.viewLatLonAltBox();

    qCDebug(MapLog) << "GeoBin at" << center().toString() << "has" << size() << "images.";
    if (viewPortParams.resolves(boundingRegion(), MARKER_SIZE_PX)
        || (boundingRegion().isNull() && viewPortParams.angularResolution() < FINE_RESOLUTION)) {
        qCDebug(MapLog) << "GeoBin: drawing individual images";
        // if the region takes up enough screen space, we should display the images.
        // if all images have the same coordinates (null bounding region), this will never happen
        // -> in this case, show the images when we're zoomed in enough
        for (const DB::ImageInfoPtr &image : m_images) {
            const Marble::GeoDataCoordinates pos(image->coordinates().lon(), image->coordinates().lat(),
                                                 image->coordinates().alt(),
                                                 Marble::GeoDataCoordinates::Degree);
            if (viewPort.contains(pos)) {
                if (style == MapStyle::ShowPins) {
                    painter->drawPixmap(pos, alternatePixmap);
                } else {
                    // FIXME(l3u) Maybe we should cache the scaled thumbnails?
                    painter->drawPixmap(pos, ImageManager::ThumbnailCache::instance()->lookup(image->fileName()).scaled(QSize(MARKER_SIZE_PX, MARKER_SIZE_PX), Qt::KeepAspectRatio));
                }
            }
        }
    } else {
        painter->setOpacity(0.5);
        if (viewPortParams.angularResolution() < FINE_RESOLUTION) {
            qCDebug(MapLog) << "GeoBin: drawing area";
            // high resolution -> draw in geo coordinates to represent the area of the images
            // the size was empirically determined
            qreal sizeDeg = 1.5 * (boundingRegion().width(Marble::GeoDataCoordinates::Degree) + boundingRegion().height(Marble::GeoDataCoordinates::Degree));
            // sometimes, the boundingRegion is much smaller than the 40px circle
            sizeDeg = qMax(sizeDeg, FINE_RESOLUTION);
            // true -> size is in degree, not screen coordinates
            painter->drawEllipse(center(), sizeDeg, sizeDeg, true);
        } else {
            qCDebug(MapLog) << "GeoBin: drawing marker";
            // low resolution -> draw in screen coordinates to keep the region visible
            painter->drawEllipse(center(), MARKER_SIZE_PX, MARKER_SIZE_PX);
        }
        painter->setOpacity(1);
        QPen pen = painter->pen();
        painter->setPen(QPen(Qt::black));
        painter->drawText(center(), i18nc("The number of images in an area of the map", "%1", size()), -0.5 * MARKER_SIZE_PX, 0.5 * MARKER_SIZE_PX, MARKER_SIZE_PX, MARKER_SIZE_PX, QTextOption(Qt::AlignCenter));
        painter->setPen(pen);
    }
}

int Map::GeoBin::size() const
{
    return m_images.size();
}

Map::MapView::MapView(QWidget *parent, UsageType type)
    : QWidget(parent)
{
    if (type == UsageType::MapViewWindow) {
        setWindowFlags(Qt::Window);
        setAttribute(Qt::WA_DeleteOnClose);
    }

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_statusLabel = new QLabel;
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->hide();
    layout->addWidget(m_statusLabel);

    m_mapWidget = new Marble::MarbleWidget;
    m_mapWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mapWidget->setProjection(Marble::Mercator);
    m_mapWidget->setMapThemeId(QStringLiteral("earth/openstreetmap/openstreetmap.dgml"));
#ifdef MARBLE_HAS_regionSelected_NEW
    connect(m_mapWidget, &Marble::MarbleWidget::regionSelected,
            this, &Map::MapView::updateRegionSelection);
#else
    connect(m_mapWidget, &Marble::MarbleWidget::regionSelected,
            this, &Map::MapView::updateRegionSelectionOld);
#endif

    m_mapWidget->addLayer(this);

    layout->addWidget(m_mapWidget);
    m_mapWidget->show();

    QHBoxLayout *controlLayout = new QHBoxLayout;
    layout->addLayout(controlLayout);

    // KPA's control buttons

    m_kpaButtons = new QWidget;
    QHBoxLayout *kpaButtonsLayout = new QHBoxLayout(m_kpaButtons);
    controlLayout->addWidget(m_kpaButtons);

    QPushButton *saveButton = new QPushButton;
    saveButton->setFlat(true);
    saveButton->setIcon(QPixmap(SmallIcon(QStringLiteral("media-floppy"))));
    saveButton->setToolTip(i18n("Save the current map settings"));
    kpaButtonsLayout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &MapView::saveSettings);

    m_setLastCenterButton = new QPushButton;
    m_setLastCenterButton->setFlat(true);
    m_setLastCenterButton->setIcon(QPixmap(SmallIcon(QStringLiteral("go-first"))));
    m_setLastCenterButton->setToolTip(i18n("Go to last map position"));
    kpaButtonsLayout->addWidget(m_setLastCenterButton);
    connect(m_setLastCenterButton, &QPushButton::clicked, this, &MapView::setLastCenter);

    QPushButton *showThumbnails = new QPushButton;
    showThumbnails->setFlat(true);
    showThumbnails->setIcon(QPixmap(SmallIcon(QStringLiteral("view-preview"))));
    showThumbnails->setToolTip(i18n("Show thumbnails"));
    kpaButtonsLayout->addWidget(showThumbnails);
    showThumbnails->setCheckable(true);
    showThumbnails->setChecked(m_showThumbnails);
    connect(showThumbnails, &QPushButton::clicked, this, &MapView::setShowThumbnails);

    // Marble floater control buttons

    m_floaters = new QWidget;
    QHBoxLayout *floatersLayout = new QHBoxLayout(m_floaters);
    controlLayout->addStretch();
    controlLayout->addWidget(m_floaters);

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("MapView"));

    for (const Marble::RenderPlugin *plugin : m_mapWidget->renderPlugins()) {
        if (plugin->renderType() != Marble::RenderPlugin::PanelRenderType) {
            continue;
        }

        const QString name = plugin->name();
        if (!WANTED_FLOATERS.contains(name)) {
            continue;
        }

        QPushButton *button = new QPushButton;
        button->setCheckable(true);
        button->setFlat(true);
        button->setChecked(plugin->action()->isChecked());
        button->setToolTip(plugin->description());
        button->setProperty("floater", name);

        QPixmap icon = plugin->action()->icon().pixmap(QSize(20, 20));
        if (icon.isNull()) {
            icon = QPixmap(20, 20);
            icon.fill(Qt::white);
        }
        button->setIcon(icon);

        connect(plugin->action(), &QAction::toggled, button, &QPushButton::setChecked);
        connect(button, &QPushButton::toggled, plugin->action(), &QAction::setChecked);
        floatersLayout->addWidget(button);
        const QVariant checked = group.readEntry(MAPVIEW_FLOATER_VISIBLE_CONFIG_PREFIX + name,
                                                 true);
        button->setChecked(checked.toBool());
    }

    m_pin = QPixmap(QStandardPaths::locate(QStandardPaths::DataLocation, QStringLiteral("pics/pin.png")));
}

void Map::MapView::clear()
{
    m_markersBox.clear();
    m_baseBins.clear();
    m_geoClusters.clear();
    m_regionSelected = false;
}

bool Map::MapView::addImage(DB::ImageInfoPtr image)
{
    if (image->coordinates().hasCoordinates()) {
        const GeoBinAddress binAddress = computeBinAddress(image->coordinates());
        if (!m_baseBins.contains(binAddress)) {
            m_baseBins.insert(binAddress, new GeoBin());
        }
        m_baseBins[binAddress]->addImage(image);
        // Update the viewport for zoomToMarkers()
        extendGeoDataLatLonBox(m_markersBox, image->coordinates());
        return true;
    }
    return false;
}

void Map::MapView::addImages(const DB::ImageSearchInfo &searchInfo)
{
    QElapsedTimer timer;
    timer.start();
    displayStatus(MapStatus::Loading);
    DB::FileNameList images = DB::ImageDB::instance()->search(searchInfo);
    int count = 0;
    int total = 0;
    // put images in bins
    for (const auto &imageInfo : images) {
        total++;
        if (addImage(imageInfo.info()))
            count++;
    }
    buildImageClusters();
    displayStatus(MapStatus::SearchCoordinates);
    qCInfo(TimingLog) << "MapView::addImages(): added" << count << "of" << total << "images in" << timer.elapsed() << "ms.";
}

void Map::MapView::buildImageClusters()
{
    QElapsedTimer timer;
    timer.start();
    QVector<QHash<GeoBinAddress, GeoCluster *>> clusters { MAP_CLUSTER_LEVELS };
    int count = 0;
    // aggregate bins to clusters
    for (auto it = m_baseBins.constBegin(); it != m_baseBins.constEnd(); ++it) {
        buildClusterMap(clusters, it.key(), it.value());
        count++;
    }
    Q_ASSERT(clusters[MAP_CLUSTER_LEVELS - 1].size() > 0);
    for (int lvl = 0; lvl < MAP_CLUSTER_LEVELS; lvl++) {
        qCInfo(MapLog) << "MapView:" << clusters[lvl].size() << "clusters on level" << lvl;
    }
    m_geoClusters = clusters[MAP_CLUSTER_LEVELS - 1];
    qCDebug(TimingLog) << "MapView::addImages(): aggregated" << count << "GeoClusters in" << timer.elapsed() << "ms.";
}

void Map::MapView::zoomToMarkers()
{
    m_mapWidget->centerOn(m_markersBox);
}

void Map::MapView::setCenter(const DB::ImageInfoPtr image)
{
    m_lastCenter = image->coordinates();
    setLastCenter();
}

void Map::MapView::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("MapView"));
    for (const QPushButton *button : m_floaters->findChildren<QPushButton *>()) {
        group.writeEntry(MAPVIEW_FLOATER_VISIBLE_CONFIG_PREFIX
                             + button->property("floater").toString(),
                         button->isChecked());
    }
    config->sync();
    QMessageBox::information(this, i18n("Map view"), i18n("Settings saved!"));
}

void Map::MapView::setShowThumbnails(bool state)
{
    m_showThumbnails = state;
    m_mapWidget->reloadMap();
}

void Map::MapView::displayStatus(MapStatus status)
{
    switch (status) {
    case MapStatus::Loading:
        m_statusLabel->setText(i18n("<i>Loading coordinates from the images ...</i>"));
        m_statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_statusLabel->show();
        m_mapWidget->hide();
        m_regionSelected = false;
        m_setLastCenterButton->setEnabled(false);
        break;
    case MapStatus::ImageHasCoordinates:
        m_statusLabel->hide();
        m_regionSelected = false;
        m_statusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        m_mapWidget->show();
        m_setLastCenterButton->show();
        m_setLastCenterButton->setEnabled(true);
        break;
    case MapStatus::ImageHasNoCoordinates:
        m_statusLabel->setText(i18n("<i>This image does not contain geographic coordinates.</i>"));
        m_statusLabel->show();
        m_statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_mapWidget->hide();
        m_setLastCenterButton->show();
        m_setLastCenterButton->setEnabled(false);
        break;
    case MapStatus::SomeImagesHaveNoCoordinates:
        m_statusLabel->setText(i18n("<i>Some of the selected images do not contain geographic "
                                    "coordinates.</i>"));
        m_statusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        m_statusLabel->show();
        m_regionSelected = false;
        m_mapWidget->show();
        m_setLastCenterButton->show();
        m_setLastCenterButton->setEnabled(true);
        break;
    case MapStatus::SearchCoordinates:
        m_statusLabel->setText(i18n("<i>Search for geographic coordinates.</i>"));
        m_statusLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        m_statusLabel->show();
        m_mapWidget->show();
        m_mapWidget->centerOn(0.0, 0.0);
        m_setLastCenterButton->hide();
        break;
    case MapStatus::NoImagesHaveNoCoordinates:
        m_statusLabel->setText(i18n("<i>None of the selected images contain geographic "
                                    "coordinates.</i>"));
        m_statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_statusLabel->show();
        m_mapWidget->hide();
        m_setLastCenterButton->show();
        m_setLastCenterButton->setEnabled(false);
        break;
    }
    emit displayStatusChanged(status);
}

void Map::MapView::setLastCenter()
{
    m_mapWidget->centerOn(m_lastCenter.lon(), m_lastCenter.lat());
}

void Map::MapView::updateRegionSelection(const Marble::GeoDataLatLonBox &selection)
{
    m_regionSelected = true;
    m_regionSelection = selection;
    emit newRegionSelected(getRegionSelection());
}

#ifndef MARBLE_HAS_regionSelected_NEW
void Map::MapView::updateRegionSelectionOld(const QList<double> &selection)
{
    Q_ASSERT(selection.length() == 4);
    // see also: https://cgit.kde.org/marble.git/commit/?id=ec1f7f554e9f6ca248b4a3b01dbf08507870687e
    Marble::GeoDataLatLonBox sel { selection.at(1), selection.at(3), selection.at(2), selection.at(0), Marble::GeoDataCoordinates::Degree };
    updateRegionSelection(sel);
}
#endif

Map::GeoCoordinates::LatLonBox Map::MapView::getRegionSelection() const
{
    return GeoCoordinates::LatLonBox(
        m_regionSelection.north(Marble::GeoDataCoordinates::Degree),
        m_regionSelection.south(Marble::GeoDataCoordinates::Degree),
        m_regionSelection.east(Marble::GeoDataCoordinates::Degree),
        m_regionSelection.west(Marble::GeoDataCoordinates::Degree));
}

bool Map::MapView::regionSelected() const
{
    return m_regionSelected;
}

QStringList Map::MapView::renderPosition() const
{
    // we only ever paint on the same layer:
    return MAPVIEW_RENDER_POSITION;
}

bool Map::MapView::render(Marble::GeoPainter *painter, Marble::ViewportParams *viewPortParams,
                          const QString &renderPos, Marble::GeoSceneLayer *)
{
    Q_ASSERT(renderPos == renderPosition().first());
    Q_ASSERT(viewPortParams != nullptr);
    QElapsedTimer timer;
    timer.start();

    painter->setBrush(QBrush(QColor(Qt::red).lighter()));
    painter->setPen(QColor(Qt::red));
    for (const auto *bin : m_geoClusters) {
        bin->render(painter, *viewPortParams, m_pin, m_showThumbnails ? MapStyle::ShowThumbnails : MapStyle::ShowPins);
    }

    qCDebug(TimingLog) << "Map rendered in" << timer.elapsed() << "ms.";
    return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

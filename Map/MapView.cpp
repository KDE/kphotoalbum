// SPDX-FileCopyrightText: 2014-2015 Tobias Leupold <tobias.leupold@gmx.de>
// SPDX-FileCopyrightText: 2015-2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "MapView.h"
#include "GeoCluster.h"
#include "Logging.h"

#include <DB/ImageDB.h>
#include <DB/ImageSearchInfo.h>
#include <MainWindow/Window.h>
#include <kpabase/Logging.h>

#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <QAction>
#include <QDebug>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QLabel>
#include <QLoggingCategory>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <marble/GeoPainter.h>
#include <marble/MarbleWidget.h>
#include <marble/RenderPlugin.h>
#include <marble/ViewportParams.h>

namespace
{
// size of the markers in screen coordinates (pixel)
constexpr int MARKER_SIZE_DEFAULT_PX = 40;
constexpr int MARKER_SIZE_MIN_PX = 20;
constexpr int MARKER_SIZE_MAX_PX = 400;
constexpr int MARKER_SIZE_STEP_PX = 10;
const QString MAPVIEW_FLOATER_VISIBLE_CONFIG_PREFIX = QStringLiteral("MarbleFloaterVisible ");
const QStringList MAPVIEW_RENDER_POSITION({ QStringLiteral("HOVERS_ABOVE_SURFACE") });
const QVector<QString> WANTED_FLOATERS { QStringLiteral("compass"),
                                         QStringLiteral("scalebar"),
                                         QStringLiteral("navigation"),
                                         QStringLiteral("overviewmap") };

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
    constexpr quint64 LO = 0x00000000ffffffff;
    constexpr quint64 HI = 0xffffffff00000000;
    // zero out the rightmost bits
    quint64 mask = 0xffffffffffffffff << level;
    // duplicate the mask onto the higher 32 bits
    mask = (HI & (mask << 32)) | (LO & mask);
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

}

namespace
{
inline QPixmap smallIcon(const QString &iconName)
{
    return QIcon::fromTheme(iconName).pixmap(KIconLoader::StdSizes::SizeSmall);
}
}

Map::MapView::MapView(QWidget *parent, UsageType type)
    : QWidget(parent)
    , m_markerSize(MARKER_SIZE_DEFAULT_PX)
{
    if (type == UsageType::MapViewWindow) {
        setWindowFlags(Qt::Window);
        setAttribute(Qt::WA_DeleteOnClose);
    }
    setMouseTracking(true);

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
    saveButton->setIcon(QPixmap(smallIcon(QStringLiteral("media-floppy"))));
    saveButton->setToolTip(i18n("Save the current map settings"));
    kpaButtonsLayout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &MapView::saveSettings);

    m_setLastCenterButton = new QPushButton;
    m_setLastCenterButton->setFlat(true);
    m_setLastCenterButton->setIcon(QPixmap(smallIcon(QStringLiteral("go-first"))));
    m_setLastCenterButton->setToolTip(i18n("Go to last map position"));
    kpaButtonsLayout->addWidget(m_setLastCenterButton);
    connect(m_setLastCenterButton, &QPushButton::clicked, this, &MapView::setLastCenter);

    QPushButton *showThumbnails = new QPushButton;
    showThumbnails->setFlat(true);
    showThumbnails->setIcon(QPixmap(smallIcon(QStringLiteral("view-preview"))));
    showThumbnails->setToolTip(i18n("Show thumbnails"));
    kpaButtonsLayout->addWidget(showThumbnails);
    showThumbnails->setCheckable(true);
    showThumbnails->setChecked(m_showThumbnails);
    connect(showThumbnails, &QPushButton::clicked, this, &MapView::setShowThumbnails);

    QPushButton *groupThumbnails = new QPushButton;
    groupThumbnails->setFlat(true);
    groupThumbnails->setIcon(QPixmap(smallIcon(QStringLiteral("view-group"))));
    groupThumbnails->setToolTip(i18nc("@action to group, as in aggregate thumbnails into clusters", "Group adjacent thumbnails into clickable clusters"));
    kpaButtonsLayout->addWidget(groupThumbnails);
    groupThumbnails->setCheckable(true);
    groupThumbnails->setChecked(m_groupThumbnails);
    connect(groupThumbnails, &QPushButton::clicked, this, &MapView::setGroupThumbnails);
    // groupThumbnails interacts with showThumbnails:
    showThumbnails->setEnabled(m_groupThumbnails);
    connect(groupThumbnails, &QPushButton::clicked, showThumbnails, &QPushButton::setEnabled);

    QPushButton *decreaseMarkerSize = new QPushButton;
    decreaseMarkerSize->setFlat(true);
    decreaseMarkerSize->setIcon(QPixmap(smallIcon(QStringLiteral("view-zoom-out-symbolic"))));
    decreaseMarkerSize->setToolTip(i18nc("@action Decrease size of the markers (i.e. thumbnails) drawn on the map", "Decrease marker size"));
    kpaButtonsLayout->addWidget(decreaseMarkerSize);
    connect(decreaseMarkerSize, &QPushButton::clicked, this, &MapView::decreaseMarkerSize);

    QPushButton *increaseMarkerSize = new QPushButton;
    increaseMarkerSize->setFlat(true);
    increaseMarkerSize->setIcon(QPixmap(smallIcon(QStringLiteral("view-zoom-in-symbolic"))));
    increaseMarkerSize->setToolTip(i18nc("@action Increase size of the markers (i.e. thumbnails) drawn on the map", "Increase marker size"));
    kpaButtonsLayout->addWidget(increaseMarkerSize);
    connect(increaseMarkerSize, &QPushButton::clicked, this, &MapView::increaseMarkerSize);

    // Marble floater control buttons

    m_floaters = new QWidget;
    QHBoxLayout *floatersLayout = new QHBoxLayout(m_floaters);
    controlLayout->addStretch();
    controlLayout->addWidget(m_floaters);

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("MapView"));

    const auto renderPlugins = m_mapWidget->renderPlugins();
    for (const Marble::RenderPlugin *plugin : renderPlugins) {
        if (plugin->renderType() != Marble::RenderPlugin::PanelRenderType) {
            continue;
        }

        const QString name = plugin->nameId();
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
            icon.fill(palette().button().color());
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
    const auto images = DB::ImageDB::instance()->search(searchInfo);
    int count = 0;
    int total = 0;
    // put images in bins
    for (const auto &imageInfo : images) {
        total++;
        if (addImage(imageInfo))
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
    // alternative proposal:
    // 1. sort clusters by size
    // 2. take biggest cluster and compute distance to other clusters
    // 3. create new aggregate cluster of biggest cluster with all clusters nearer than 1km*2^level
    //    remove aggregated clusters from set of eligible clusters
    // 4. with remaining clusters, continue at 2.

    Q_ASSERT(clusters[MAP_CLUSTER_LEVELS - 1].size() > 0 || clusters[0].size() == 0);
    for (int lvl = 0; lvl < MAP_CLUSTER_LEVELS; lvl++) {
        qCInfo(MapLog) << "MapView:" << clusters[lvl].size() << "clusters on level" << lvl;
    }
    m_geoClusters = clusters[MAP_CLUSTER_LEVELS - 1].values();
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

void Map::MapView::increaseMarkerSize()
{
    setMarkerSize(markerSize() + MARKER_SIZE_STEP_PX);
}

void Map::MapView::decreaseMarkerSize()
{
    setMarkerSize(markerSize() - MARKER_SIZE_STEP_PX);
}

void Map::MapView::saveSettings()
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("MapView"));
    const auto buttons = m_floaters->findChildren<QPushButton *>();
    for (const QPushButton *button : buttons) {
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
    m_mapWidget->update();
}

void Map::MapView::setGroupThumbnails(bool state)
{
    m_groupThumbnails = state;
    m_mapWidget->update();
}

Map::MapStyle Map::MapView::mapStyle() const
{
    if (m_groupThumbnails)
        return m_showThumbnails ? MapStyle::ShowThumbnails : MapStyle::ShowPins;
    else
        return MapStyle::ForceShowThumbnails;
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

int Map::MapView::markerSize() const
{
    return m_markerSize;
}

void Map::MapView::setMarkerSize(int markerSizePx)
{
    m_markerSize = qBound(MARKER_SIZE_MIN_PX, markerSizePx, MARKER_SIZE_MAX_PX);
    qCDebug(MapLog) << "Set map marker size to" << m_markerSize;
    m_mapWidget->update();
}

#ifndef MARBLE_HAS_regionSelected_NEW
void Map::MapView::updateRegionSelectionOld(const QList<double> &selection)
{
    Q_ASSERT(selection.length() == 4);
    // see also: https://commits.kde.org/marble/ec1f7f554e9f6ca248b4a3b01dbf08507870687e
    Marble::GeoDataLatLonBox sel { selection.at(1), selection.at(3), selection.at(2), selection.at(0), Marble::GeoDataCoordinates::Degree };
    updateRegionSelection(sel);
}
#endif

Map::GeoCoordinates::LatLonBox Map::MapView::getRegionSelection() const
{
    return GeoCoordinates::LatLonBox(m_regionSelection);
}

bool Map::MapView::regionSelected() const
{
    return m_regionSelected;
}

void Map::MapView::mousePressEvent(QMouseEvent *event)
{
    if (m_preselectedCluster && event->button() == Qt::RightButton) {
        // cancel geocluster selection if RMB is clicked
        m_preselectedCluster = nullptr;
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) {
        if (m_mapWidget->geometry().contains(event->pos())) {
            qCDebug(MapLog) << "Map clicked.";
            const auto mapPos = event->pos() - m_mapWidget->pos();

            for (const auto *topLevelCluster : qAsConst(m_geoClusters)) {
                const auto subCluster = topLevelCluster->regionForPoint(mapPos);
                if (subCluster && !subCluster->isEmpty()) {
                    qCDebug(MapLog) << "Cluster preselected/clicked.";
                    m_preselectedCluster = subCluster;
                    event->accept();
                    return;
                }
            }
        }
    }
    event->ignore();
    QWidget::mousePressEvent(event);
}

void Map::MapView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_preselectedCluster) {
        qCDebug(MapLog) << "Cluster selection accepted.";
        updateRegionSelection(m_preselectedCluster->boundingRegion());
        m_preselectedCluster = nullptr;
        event->accept();
    } else {
        QWidget::mouseReleaseEvent(event);
    }
}

void Map::MapView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() == Qt::NoButton) {
        if (m_mapWidget->geometry().contains(event->pos())) {
            const auto mapPos = event->pos() - m_mapWidget->pos();
            for (const auto *topLevelCluster : qAsConst(m_geoClusters)) {
                const auto subCluster = topLevelCluster->regionForPoint(mapPos);
                // Note(jzarl) unfortunately we cannot use QWidget::setCursor here
                if (subCluster) {
                    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
                } else {
                    QGuiApplication::restoreOverrideCursor();
                }
            }
        }
    }
    QWidget::mouseMoveEvent(event);
}

void Map::MapView::keyPressEvent(QKeyEvent *event)
{
    if (m_preselectedCluster && event->matches(QKeySequence::Cancel)) {
        // cancel geocluster selection if Escape key is pressed
        m_preselectedCluster = nullptr;
    } else {
        QWidget::keyPressEvent(event);
    }
}

QStringList Map::MapView::renderPosition() const
{
    // we only ever paint on the same layer:
    return MAPVIEW_RENDER_POSITION;
}

bool Map::MapView::render(Marble::GeoPainter *painter, Marble::ViewportParams *viewPortParams,
                          const QString &renderPos, Marble::GeoSceneLayer *)
{
    Q_ASSERT(renderPos == renderPosition().constFirst());
    Q_ASSERT(viewPortParams != nullptr);
    QElapsedTimer timer;
    timer.start();

    painter->setBrush(Qt::transparent);
    painter->setPen(palette().color(QPalette::Highlight));
    constexpr qreal BOUNDING_BOX_SCALEFACTOR = 1.2;
    painter->drawRect(m_markersBox.center(), m_markersBox.width(Marble::GeoDataCoordinates::Degree) * BOUNDING_BOX_SCALEFACTOR, m_markersBox.height(Marble::GeoDataCoordinates::Degree) * BOUNDING_BOX_SCALEFACTOR, true);

    painter->setBrush(palette().brush(QPalette::Dark));
    painter->setPen(palette().color(QPalette::Text));
    ThumbnailParams thumbs { m_pin, MainWindow::Window::theMainWindow()->thumbnailCache(), m_markerSize };
    for (const auto *bin : qAsConst(m_geoClusters)) {
        bin->render(painter, *viewPortParams, thumbs, mapStyle());
    }
    if (m_preselectedCluster) {
        painter->setBrush(palette().brush(QPalette::Highlight));
        painter->setPen(palette().color(QPalette::HighlightedText));
        m_preselectedCluster->render(painter, *viewPortParams, thumbs, mapStyle());
    }

    qCDebug(TimingLog) << "Map rendered in" << timer.elapsed() << "ms.";
    return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

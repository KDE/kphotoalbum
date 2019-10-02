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
}

void Map::GeoBin::addImage(DB::ImageInfoPtr image)
{
    m_images.append(image);
}

Map::GeoCoordinates Map::GeoBin::center() const
{
    // FIXME(jzarl): this is just a STUB until the clustering works
    return m_images.first()->coordinates();
}

void Map::GeoBin::render(Marble::GeoPainter *painter, const Marble::GeoDataLatLonAltBox &viewPort, const QPixmap &alternatePixmap, MapStyle style) const
{
    for (const DB::ImageInfoPtr &image : m_images) {
        const Marble::GeoDataCoordinates pos(image->coordinates().lon(), image->coordinates().lat(),
                                             image->coordinates().alt(),
                                             Marble::GeoDataCoordinates::Degree);
        if (viewPort.contains(pos)) {
            if (style == MapStyle::ShowPins) {
                painter->drawPixmap(pos, alternatePixmap);
            } else {
                // FIXME(l3u) Maybe we should cache the scaled thumbnails?
                painter->drawPixmap(pos, ImageManager::ThumbnailCache::instance()->lookup(image->fileName()).scaled(QSize(40, 40), Qt::KeepAspectRatio));
            }
        }
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
    m_geoBins.clear();
    m_markersBox.clear();
    m_regionSelected = false;
}

void Map::MapView::addImage(DB::ImageInfoPtr image)
{
    qCDebug(MapLog) << "Adding image" << image->label();
    const GeoBinAddress binAddress = computeBinAddress(image->coordinates());
    m_geoBins[binAddress].addImage(image);

    // Update the viewport for zoomToMarkers()
    if (m_markersBox.isEmpty()) {
        m_markersBox.setEast(image->coordinates().lon(), Marble::GeoDataCoordinates::Degree);
        m_markersBox.setWest(image->coordinates().lon(), Marble::GeoDataCoordinates::Degree);
        m_markersBox.setNorth(image->coordinates().lat(), Marble::GeoDataCoordinates::Degree);
        m_markersBox.setSouth(image->coordinates().lat(), Marble::GeoDataCoordinates::Degree);
    } else {
        if (m_markersBox.east(Marble::GeoDataCoordinates::Degree) < image->coordinates().lon()) {
            m_markersBox.setEast(image->coordinates().lon(), Marble::GeoDataCoordinates::Degree);
        }
        if (m_markersBox.west(Marble::GeoDataCoordinates::Degree) > image->coordinates().lon()) {
            m_markersBox.setWest(image->coordinates().lon(), Marble::GeoDataCoordinates::Degree);
        }
        if (m_markersBox.north(Marble::GeoDataCoordinates::Degree) < image->coordinates().lat()) {
            m_markersBox.setNorth(image->coordinates().lat(), Marble::GeoDataCoordinates::Degree);
        }
        if (m_markersBox.south(Marble::GeoDataCoordinates::Degree) > image->coordinates().lat()) {
            m_markersBox.setSouth(image->coordinates().lat(), Marble::GeoDataCoordinates::Degree);
        }
    }
}

void Map::MapView::addImages(const DB::ImageSearchInfo &searchInfo)
{
    QElapsedTimer timer;
    timer.start();
    displayStatus(MapStatus::Loading);
    DB::FileNameList images = DB::ImageDB::instance()->search(searchInfo);
    int count = 0;
    int total = 0;
    for (const auto &imageInfo : images) {
        DB::ImageInfoPtr image = imageInfo.info();
        total++;
        if (image->coordinates().hasCoordinates()) {
            count++;
            addImage(image);
        }
    }
    displayStatus(MapStatus::SearchCoordinates);
    qCDebug(TimingLog) << "MapView::addImages(): added" << count << "of" << total << "images in"
                       << timer.elapsed() << "ms.";
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

Map::GeoCoordinates::Pair Map::MapView::getRegionSelection() const
{
    return GeoCoordinates::makePair(m_regionSelection.west(Marble::GeoDataCoordinates::Degree),
                                    m_regionSelection.north(Marble::GeoDataCoordinates::Degree),
                                    m_regionSelection.east(Marble::GeoDataCoordinates::Degree),
                                    m_regionSelection.south(Marble::GeoDataCoordinates::Degree));
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
    int numDisplayed = 0;
    int numBins = 0;

    const auto viewPort = viewPortParams->viewLatLonAltBox();

    for (const auto &bin : m_geoBins) {
        numBins++;
        numDisplayed += bin.size();
        bin.render(painter, viewPort, m_pin, m_showThumbnails ? MapStyle::ShowThumbnails : MapStyle::ShowPins);
    }

    qCDebug(TimingLog) << "Map rendered" << numBins << "bins containing" << numDisplayed << "images in"
                       << timer.elapsed() << "ms.";
    return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

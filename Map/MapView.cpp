/* Copyright (C) 2014 Tobias Leupold <tobias.leupold@web.de>

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

#include "MapView.h"

// Qt includes
#include <QVBoxLayout>
#include <QPushButton>
#include <QPixmap>
#include <QProgressBar>
#include <QLabel>

// KDE includes
#include <KLocale>
#include <KIconLoader>
#include <KConfigGroup>
#include <KMessageBox>

// Local includes
#include "MapMarkerModelHelper.h"

Map::MapView::MapView(QWidget* parent, UsageType type) : QWidget(parent)
{
    if (type == MapViewWindow) {
        setWindowFlags(Qt::Window);
        setAttribute(Qt::WA_DeleteOnClose);
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    m_statusLabel = new QLabel;
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_statusLabel->hide();
    layout->addWidget(m_statusLabel);

    m_mapWidget = new KGeoMap::KGeoMapWidget(this);
    layout->addWidget(m_mapWidget);

    QWidget* controlWidget = m_mapWidget->getControlWidget();
    controlWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layout->addWidget(controlWidget);
    m_mapWidget->setActive(true);

    QPushButton* saveButton = new QPushButton;
    saveButton->setIcon(QPixmap(SmallIcon(QString::fromUtf8("media-floppy"))));
    saveButton->setToolTip(i18n("Save the current map settings"));
    m_mapWidget->addWidgetToControlWidget(saveButton);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(saveSettings()));

    m_setLastCenterButton = new QPushButton;
    m_setLastCenterButton->setIcon(QPixmap(SmallIcon(QString::fromUtf8("go-first"))));
    m_setLastCenterButton->setToolTip(i18n("Go to last map position"));
    m_mapWidget->addWidgetToControlWidget(m_setLastCenterButton);
    connect(m_setLastCenterButton, SIGNAL(clicked()), this, SLOT(setLastCenter()));

    // We first try set the default backend "marble" or the first one available ...
    const QString defaultBackend = QString::fromUtf8("marble");
    auto backends = m_mapWidget->availableBackends();
    if (backends.contains(defaultBackend)) {
        m_mapWidget->setBackend(defaultBackend);
    } else {
        qDebug() << "AnnotationMap: using backend " << backends[0];
        m_mapWidget->setBackend(backends[0]);
    }

    // ... then we try to set the (probably) saved settings
    KConfigGroup configGroup = KGlobal::config()->group(QString::fromUtf8("MapView"));
    m_mapWidget->readSettingsFromGroup(&configGroup);

    // Add the item model for the coordinates display
    m_modelHelper = new MapMarkerModelHelper();
    m_itemMarkerTiler = new KGeoMap::ItemMarkerTiler(m_modelHelper, this);
    m_mapWidget->setGroupedModel(m_itemMarkerTiler);
}

Map::MapView::~MapView()
{
    delete m_modelHelper;
    delete m_itemMarkerTiler;
}

void Map::MapView::clear()
{
    m_modelHelper->clearItems();
}

void Map::MapView::addImage(const DB::ImageInfo& image)
{
    m_modelHelper->addImage(image);
}

void Map::MapView::addImage(const DB::ImageInfoPtr image)
{
    m_modelHelper->addImage(image);
}

void Map::MapView::zoomToMarkers()
{
    m_mapWidget->adjustBoundariesToGroupedMarkers();
    m_lastCenter = m_mapWidget->getCenter();
}

void Map::MapView::setCenter(const DB::ImageInfo& image)
{
    m_lastCenter = image.coordinates();
    m_mapWidget->setCenter(m_lastCenter);
}

void Map::MapView::setCenter(const DB::ImageInfoPtr image)
{
    m_lastCenter = image->coordinates();
    m_mapWidget->setCenter(m_lastCenter);
}

void Map::MapView::saveSettings()
{
    KSharedConfigPtr config = KGlobal::config();
    KConfigGroup configGroup = config->group(QString::fromUtf8("MapView"));
    m_mapWidget->saveSettingsToGroup(&configGroup);
    config->sync();
    KMessageBox::information(this, i18n("Settings saved"), i18n("Map view"));
}

void Map::MapView::setShowThumbnails(bool state)
{
    m_mapWidget->setShowThumbnails(state);
}

void Map::MapView::displayStatus(MapStatus status)
{
    if (status == MapStatus::Loading) {
        m_statusLabel->setText(i18n("<i>Loading coordinates from the images ...</i>"));
        m_statusLabel->show();
        m_mapWidget->hide();
        m_setLastCenterButton->setEnabled(false);
    } else if (status == MapStatus::ImageHasCoordinates) {
        m_statusLabel->hide();
        m_mapWidget->show();
        m_setLastCenterButton->setEnabled(true);
    } else if (status == MapStatus::ImageHasNoCoordinates) {
        m_statusLabel->setText(i18n("<i>This image does not contain geographic coordinates.</i>"));
        m_statusLabel->show();
        m_mapWidget->hide();
        m_setLastCenterButton->setEnabled(false);
    } else if (status == MapStatus::SomeImagesHaveNoCoordinates) {
        m_statusLabel->setText(i18n("<i>Some of the selected images do not contain geographic "
                                    "coordinates.</i>"));
        m_statusLabel->show();
        m_mapWidget->show();
        m_setLastCenterButton->setEnabled(true);
    } else if (status == MapStatus::NoImagesHaveNoCoordinates) {
        m_statusLabel->setText(i18n("<i>None of the selected images contain geographic "
                                    "coordinates.</i>"));
        m_statusLabel->show();
        m_mapWidget->hide();
        m_setLastCenterButton->setEnabled(false);
    }
}

void Map::MapView::setLastCenter()
{
    m_mapWidget->setCenter(m_lastCenter);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

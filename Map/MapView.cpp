/* Copyright (C) 2014-2018 Tobias Leupold <tobias.leupold@gmx.de>

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

// Local includes
#include "MapView.h"

#include "Logging.h"

// Marble includes
#include <marble/MarbleWidget.h>

// Qt includes
#include <QLabel>
#include <QLoggingCategory>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>

// KDE includes
#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

Map::MapView::MapView(QWidget *parent, UsageType type)
    : QWidget(parent)
{
    if (type == MapViewWindow) {
        setWindowFlags(Qt::Window);
        setAttribute(Qt::WA_DeleteOnClose);
    }

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_statusLabel = new QLabel;
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    m_statusLabel->hide();
    layout->addWidget(m_statusLabel);

    m_mapWidget = new Marble::MarbleWidget;
    m_mapWidget->setProjection(Marble::Mercator);
    m_mapWidget->setMapThemeId( QString::fromUtf8( "earth/openstreetmap/openstreetmap.dgml" ) );
    layout->addWidget(m_mapWidget);

    QWidget *controlWidget = new QWidget( this );
    controlWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    layout->addWidget(controlWidget);

    m_mapWidget->show();

    QPushButton *saveButton = new QPushButton;
    saveButton->setIcon(QPixmap(SmallIcon(QString::fromUtf8("media-floppy"))));
    saveButton->setToolTip(i18n("Save the current map settings"));
    //m_mapWidget->addWidgetToControlWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &MapView::saveSettings);

    m_setLastCenterButton = new QPushButton;
    m_setLastCenterButton->setIcon( QPixmap( SmallIcon( QString::fromUtf8( "go-first" ) ) ) );
    m_setLastCenterButton->setToolTip(i18n("Go to last map position"));
    //m_mapWidget->addWidgetToControlWidget(m_setLastCenterButton);
    connect(m_setLastCenterButton, &QPushButton::clicked, this, &MapView::setLastCenter);

    //connect(m_mapWidget, &KGeoMap::MapWidget::signalRegionSelectionChanged,
    //        this, &MapView::signalRegionSelectionChanged);
}

Map::MapView::~MapView()
{
}

void Map::MapView::clear()
{
    qDebug() << ">>> Implement me! Map::MapView::clear()";
}

void Map::MapView::addImage(const DB::ImageInfo &image)
{
    Q_UNUSED( image );
    qDebug() << ">>> Implement me! Map::MapView::addImage(const DB::ImageInfo& image)";
}

void Map::MapView::addImage( const DB::ImageInfoPtr image )
{
    Q_UNUSED( image )
    qDebug() << ">>> Implement me! Map::MapView::addImage(const DB::ImageInfoPtr image)";
}

void Map::MapView::zoomToMarkers()
{
    qDebug() << ">>> Implement me! Map::MapView::zoomToMarkers()";
}

void Map::MapView::setCenter(const DB::ImageInfo &image)
{
    m_lastCenter = image.coordinates();
    qDebug() << ">>> Implement me! Map::MapView::setCenter(const DB::ImageInfo &image)";
}

void Map::MapView::setCenter(const DB::ImageInfoPtr image)
{
    m_lastCenter = image->coordinates();
    qDebug() << ">>> Implement me! Map::MapView::setCenter(const DB::ImageInfoPtr image)";
}

void Map::MapView::saveSettings()
{
    qDebug() << ">>> Implement me! Map::MapView::saveSettings()";
}

void Map::MapView::setShowThumbnails(bool state)
{
    Q_UNUSED( state );
    qDebug() << ">>> Implement me! Map::MapView::setShowThumbnails(bool state)";
}

void Map::MapView::displayStatus(MapStatus status)
{
    switch (status) {
    case MapStatus::Loading:
        m_statusLabel->setText(i18n("<i>Loading coordinates from the images ...</i>"));
        m_statusLabel->show();
        m_mapWidget->hide();
        //m_mapWidget->clearRegionSelection();
        m_setLastCenterButton->setEnabled(false);
        break;
    case MapStatus::ImageHasCoordinates:
        m_statusLabel->hide();
        //m_mapWidget->setAvailableMouseModes(KGeoMap::MouseModePan);
        //m_mapWidget->setVisibleMouseModes(0);
        //m_mapWidget->setMouseMode(KGeoMap::MouseModePan);
        //m_mapWidget->clearRegionSelection();
        m_mapWidget->show();
        m_setLastCenterButton->show();
        m_setLastCenterButton->setEnabled(true);
        break;
    case MapStatus::ImageHasNoCoordinates:
        m_statusLabel->setText(i18n("<i>This image does not contain geographic coordinates.</i>"));
        m_statusLabel->show();
        m_mapWidget->hide();
        m_setLastCenterButton->show();
        m_setLastCenterButton->setEnabled(false);
        break;
    case MapStatus::SomeImagesHaveNoCoordinates:
        m_statusLabel->setText(i18n("<i>Some of the selected images do not contain geographic "
                                    "coordinates.</i>"));
        m_statusLabel->show();
        //m_mapWidget->setAvailableMouseModes(KGeoMap::MouseModePan);
        //m_mapWidget->setVisibleMouseModes(0);
        //m_mapWidget->setMouseMode(KGeoMap::MouseModePan);
        //m_mapWidget->clearRegionSelection();
        m_mapWidget->show();
        m_setLastCenterButton->show();
        m_setLastCenterButton->setEnabled(true);
        break;
    case MapStatus::SearchCoordinates:
        m_statusLabel->setText(i18n("<i>Search for geographic coordinates.</i>"));
        m_statusLabel->show();
        //m_mapWidget->setAvailableMouseModes(KGeoMap::MouseModePan
        //                                    | KGeoMap::MouseModeRegionSelectionFromIcon
        //                                    | KGeoMap::MouseModeRegionSelection);
        //m_mapWidget->setVisibleMouseModes(KGeoMap::MouseModePan
        //                                  | KGeoMap::MouseModeRegionSelectionFromIcon
        //                                  | KGeoMap::MouseModeRegionSelection);
        //m_mapWidget->setMouseMode(KGeoMap::MouseModeRegionSelectionFromIcon);
        m_mapWidget->show();
        //m_mapWidget->setCenter(KGeoMap::GeoCoordinates());
        m_setLastCenterButton->hide();
        break;
    case MapStatus::NoImagesHaveNoCoordinates:
        m_statusLabel->setText(i18n("<i>None of the selected images contain geographic "
                                    "coordinates.</i>"));
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
    qDebug() << ">>> Implement me! Map::MapView::setLastCenter()";
}

Map::GeoCoordinates::Pair Map::MapView::getRegionSelection() const
{
    qDebug() << ">>> Implement me! Map::MapView::getRegionSelection()";
    return GeoCoordinates::makePair( 0, 0, 0, 0 );
}

bool Map::MapView::regionSelected() const
{
    qDebug() << ">>> Implement me! Map::MapView::regionSelected()";
    return false;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

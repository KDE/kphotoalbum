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
#include <marble/GeoPainter.h>
#include <marble/MarbleWidget.h>
#include <marble/RenderPlugin.h>

// Qt includes
#include <QLabel>
#include <QLoggingCategory>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QAction>

// KDE includes
#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

namespace {
const QString FLOATER_VISIBLE_CONFIG_PREFIX = QStringLiteral( "MarbleFloaterVisible " );
}

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
    m_mapWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mapWidget->setProjection(Marble::Mercator);
    m_mapWidget->setMapThemeId( QString::fromUtf8( "earth/openstreetmap/openstreetmap.dgml" ) );

    m_mapWidget->addLayer(this);

    layout->addWidget(m_mapWidget);
    m_mapWidget->show();

    QHBoxLayout *controlLayout = new QHBoxLayout;
    layout->addLayout(controlLayout);

    // KPA's control buttons

    QWidget* kpaButtons = new QWidget;
    QHBoxLayout* kpaButtonsLayout = new QHBoxLayout( kpaButtons );
    controlLayout->addWidget( kpaButtons );

    QPushButton *saveButton = new QPushButton;
    saveButton->setFlat(true);
    saveButton->setIcon(QPixmap(SmallIcon(QString::fromUtf8("media-floppy"))));
    saveButton->setToolTip(i18n("Save the current map settings"));
    kpaButtonsLayout->addWidget(saveButton);
    connect(saveButton, &QPushButton::clicked, this, &MapView::saveSettings);

    m_setLastCenterButton = new QPushButton;
    m_setLastCenterButton->setFlat(true);
    m_setLastCenterButton->setIcon( QPixmap( SmallIcon( QString::fromUtf8( "go-first" ) ) ) );
    m_setLastCenterButton->setToolTip(i18n("Go to last map position"));
    kpaButtonsLayout->addWidget(m_setLastCenterButton);
    connect(m_setLastCenterButton, &QPushButton::clicked, this, &MapView::setLastCenter);

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

        QPushButton *button = new QPushButton;
        button->setCheckable(true);
        button->setFlat(true);
        button->setChecked(plugin->action()->isChecked());
        button->setToolTip(plugin->description());
        const QString name = plugin->name();
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

        const QString value = group.readEntry( FLOATER_VISIBLE_CONFIG_PREFIX + name );
        if (! value.isEmpty()) {
            button->setChecked(value == QStringLiteral("true") ? true : false);
        }
    }
}

void Map::MapView::clear()
{
    m_images.clear();
}

void Map::MapView::addImage(DB::ImageInfoPtr image)
{
    if ( image->coordinates().hasCoordinates() ) {
        qCDebug( MapLog ) << "Adding image" << image->label();
        m_images.append( image );
    } else
        qCDebug( MapLog ) << "Image" << image->label() << "has no geo coordinates";
}

void Map::MapView::zoomToMarkers()
{
    qDebug() << ">>> Implement me! Map::MapView::zoomToMarkers()";
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
        group.writeEntry( FLOATER_VISIBLE_CONFIG_PREFIX + button->property( "floater" ).toString(),
                          button->isChecked() );
    }
    config->sync();
    QMessageBox::information(this, i18n("Map view"), i18n("Settings saved!"));
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
    m_mapWidget->centerOn(m_lastCenter.lon(), m_lastCenter.lat());
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

QStringList Map::MapView::renderPosition() const
{
    // we only ever paint on the same layer:
    return QStringList( { QStringLiteral( "HOVERS_ON_SURFACE" ) } );
}

bool Map::MapView::render( Marble::GeoPainter* painter, Marble::ViewportParams* viewport, const QString& renderPos, Marble::GeoSceneLayer* layer )
{
    Q_UNUSED( viewport );
    Q_UNUSED( renderPos );
    Q_UNUSED( layer );
    Q_ASSERT(renderPos == renderPosition().first());

    painter->setRenderHint( QPainter::Antialiasing, true );
    painter->setPen( QPen( QBrush( QColor::fromRgb( 255, 0, 0 ) ), 3.0, Qt::SolidLine, Qt::RoundCap ) );

    for ( const auto& image : m_images ) {
        auto pos = Marble::GeoDataCoordinates( image->coordinates().lon(), image->coordinates().lat(), image->coordinates().alt(), Marble::GeoDataCoordinates::Degree );
        painter->drawAnnotation( pos, image->label() );
    }

    return true;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

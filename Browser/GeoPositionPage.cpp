/* SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "GeoPositionPage.h"

#include "BrowserWidget.h"
#include "ImageViewPage.h"
#include "Logging.h"
#include "OverviewPage.h"
#include "enums.h"

#include <DB/ImageDB.h>
#include <MainWindow/Window.h>
#include <Map/GeoCoordinates.h>
#include <Map/MapView.h>

#include <KLocalizedString>

Browser::GeoPositionPage::GeoPositionPage(const DB::ImageSearchInfo &info, BrowserWidget *browser)
    : BrowserPage(info, browser)
{
    m_active = false;
}

void Browser::GeoPositionPage::activate()
{
    if (!m_active) {
        MainWindow::Window::theMainWindow()->showPositionBrowser();
        auto map = MainWindow::Window::theMainWindow()->positionBrowserWidget();
        map->clear();
        map->addImages(searchInfo());
        map->zoomToMarkers();

        connect(map, &Map::MapView::newRegionSelected, this, &GeoPositionPage::slotNewRegionSelected);
        m_active = true;
    }
}

void Browser::GeoPositionPage::deactivate()
{
    if (m_active) {
        m_active = false;
        auto map = MainWindow::Window::theMainWindow()->positionBrowserWidget();
        map->clear();
        map->disconnect(this);
    }
}

void Browser::GeoPositionPage::slotNewRegionSelected(Map::GeoCoordinates::LatLonBox coordinates)
{
    const QString name = i18n("Geo Position");
    DB::ImageSearchInfo info = searchInfo();

    info.setRegionSelection(coordinates);

    browser()->addAction(new Browser::OverviewPage(Breadcrumb(name), info, browser()));
    const int numSelected = DB::ImageDB::instance()->search(info).size();
    qCDebug(BrowserLog) << "Selected region" << coordinates << "with" << numSelected << "images.";
    if (numSelected <= Settings::SettingsData::instance()->autoShowThumbnailView()) {
        browser()->addAction(new ImageViewPage(info, browser()));
    }
}

Browser::Viewer Browser::GeoPositionPage::viewer()
{
    return ShowGeoPositionViewer;
}

bool Browser::GeoPositionPage::isSearchable() const
{
    return false;
}

bool Browser::GeoPositionPage::showDuringMovement() const
{
    return true;
}
// vi:expandtab:tabstop=4 shiftwidth=4:

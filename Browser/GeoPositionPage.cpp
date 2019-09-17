/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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

#include "GeoPositionPage.h"

#include "BrowserWidget.h"
#include "ImageViewPage.h"
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

void Browser::GeoPositionPage::slotNewRegionSelected(Map::GeoCoordinates::Pair coordinates)
{
    const QString name = i18n("Geo Position");
    DB::ImageSearchInfo info = searchInfo();

    info.setRegionSelection(coordinates);

    browser()->addAction(new Browser::OverviewPage(Breadcrumb(name), info, browser()));
    if (DB::ImageDB::instance()->search(info).size() <= Settings::SettingsData::instance()->autoShowThumbnailView()) {
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

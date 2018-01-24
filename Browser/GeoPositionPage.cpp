/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include <KLocalizedString>

Browser::GeoPositionPage::GeoPositionPage(const DB::ImageSearchInfo &info, BrowserWidget *browser)
    : BrowserPage(info, browser)
{
    active = false;
}

void Browser::GeoPositionPage::activate()
{
    if (!active) {
        MainWindow::Window::theMainWindow()->showPositionBrowser();
        Browser::PositionBrowserWidget *positionBrowserWidget = MainWindow::Window::theMainWindow()->positionBrowserWidget();
        positionBrowserWidget->showImages(searchInfo());

        connect(positionBrowserWidget, &Browser::PositionBrowserWidget::signalNewRegionSelected,
                this, &GeoPositionPage::slotNewRegionSelected);
        active = true;
    }
}

void Browser::GeoPositionPage::deactivate()
{
    if (active) {
        active = false;
        Browser::PositionBrowserWidget *positionBrowserWidget = MainWindow::Window::theMainWindow()->positionBrowserWidget();
        positionBrowserWidget->clearImages();

        disconnect(positionBrowserWidget, 0, this, 0);
    }
}

void Browser::GeoPositionPage::slotNewRegionSelected(Map::GeoCoordinates::Pair coordinates)
{
    const QString name = i18n("Geo position");
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

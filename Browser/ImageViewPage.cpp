/* Copyright (C) 2003-2020 Jesper K. Pedersen <blackie@kde.org>

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

#include "ImageViewPage.h"

#include <DB/ImageDB.h>
#include <MainWindow/Window.h>
#include <ThumbnailView/ThumbnailFacade.h>

Browser::ImageViewPage::ImageViewPage(const DB::ImageSearchInfo &info, BrowserWidget *browser)
    : BrowserPage(info, browser)
{
}

void Browser::ImageViewPage::activate()
{
    MainWindow::Window::theMainWindow()->showThumbNails(DB::ImageDB::instance()->search(searchInfo()).files());

    if (!m_context.isNull()) {
        // PENDING(blackie) this is the only place that uses the ThumbnailFacade as a singleton. Rewrite to make it communicate with it otherwise.
        ThumbnailView::ThumbnailFacade::instance()->setCurrentItem(m_context);
    }
}

Browser::Viewer Browser::ImageViewPage::viewer()
{
    return Browser::ShowImageViewer;
}

bool Browser::ImageViewPage::isSearchable() const
{
    return false;
}

Browser::ImageViewPage::ImageViewPage(const DB::FileName &context, BrowserWidget *browser)
    : BrowserPage(DB::ImageSearchInfo(), browser)
    , m_context(context)
{
}

bool Browser::ImageViewPage::showDuringMovement() const
{
    return true;
}

Browser::Breadcrumb Browser::ImageViewPage::breadcrumb() const
{
    return Breadcrumb::view();
}
// vi:expandtab:tabstop=4 shiftwidth=4:

/* SPDX-FileCopyrightText: 2003-2020 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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
    return true;
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

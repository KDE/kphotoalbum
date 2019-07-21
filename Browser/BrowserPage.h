/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef BROWSERPAGE_H
#define BROWSERPAGE_H
#include "Breadcrumb.h"

#include <DB/Category.h>
#include <DB/ImageSearchInfo.h>

class QModelIndex;
namespace Browser
{
class BrowserWidget;

enum Viewer { ShowBrowser,
              ShowImageViewer,
              ShowGeoPositionViewer };

/**
 * \brief Information about a single page in the browser
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 *
 * This interface represent a single page in the browser (one that you can go
 * back/forward to using the back/forward buttons in the toolbar).
 */
class BrowserPage : public QObject
{
    Q_OBJECT
public:
    BrowserPage(const DB::ImageSearchInfo &info, BrowserWidget *browser);
    ~BrowserPage() override {}

    /**
     * Construct the page. Result of activation may be to call \ref BrowserWidget::addAction.
     */
    virtual void activate() = 0;
    virtual void deactivate();
    virtual BrowserPage *activateChild(const QModelIndex &);
    virtual Viewer viewer();
    virtual DB::Category::ViewType viewType() const;
    virtual bool isSearchable() const;
    virtual bool isViewChangeable() const;
    virtual Breadcrumb breadcrumb() const;
    virtual bool showDuringMovement() const;

    DB::ImageSearchInfo searchInfo() const;
    BrowserWidget *browser() const;

private:
    DB::ImageSearchInfo m_info;
    BrowserWidget *m_browser;
};

}

#endif /* BROWSERPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BROWSERPAGE_H
#define BROWSERPAGE_H
#include "Breadcrumb.h"

#include <DB/Category.h>
#include <DB/search/ImageSearchInfo.h>

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
    ~BrowserPage() override { }

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

// SPDX-FileCopyrightText: 2003-2010, 2012-2013 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2005-2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2006 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2008 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2008 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2013 Dominik Broj <broj.dominik@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2022 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BROWSER_H
#define BROWSER_H
#include "BreadcrumbList.h"
#include "CenteringIconView.h"

#include <DB/Category.h>
#include <kpabase/SettingsData.h>

#include <QListView>

class CenteringIconView;
class QSortFilterProxyModel;
class QTreeView;
class QListView;
class QStackedWidget;

namespace DB
{
class ImageSearchInfo;
class FileName;
}

namespace Browser
{
class TreeFilter;
class BrowserPage;

/**
 * \brief The widget that makes up the Browser, and the interface to the other modules in KPhotoAlbum.
 *
 * See \ref Browser for a detailed description of how this fits in with the rest of the classes in this module
 */
class BrowserWidget : public QWidget
{
    Q_OBJECT
    friend class ImageFolderAction;

public:
    explicit BrowserWidget(QWidget *parent);
    void addSearch(DB::ImageSearchInfo &info);
    void addImageView(const DB::FileName &context);
    static BrowserWidget *instance();
    void load(const QString &category, const QString &value);
    DB::ImageSearchInfo currentContext();
    void setFocus();
    QString currentCategory() const;
    void addAction(Browser::BrowserPage *);
    void setModel(QAbstractItemModel *);
    static bool isResizing() { return s_isResizing; }

public Q_SLOTS:
    void back();
    void forward();
    void go();
    void home();
    void reload();
    void slotSmallListView();
    void slotLargeListView();
    void slotSmallIconView();
    void slotLargeIconView();
    void slotSortViewNaturally(bool on);
    void slotLimitToMatch(const QString &);
    void slotInvokeSeleted();
    void scrollKeyPressed(QKeyEvent *);
    void widenToBreadcrumb(const Browser::Breadcrumb &);

Q_SIGNALS:
    void canGoBack(bool);
    void canGoForward(bool);
    void showingOverview();
    void pathChanged(const Browser::BreadcrumbList &);
    void isSearchable(bool);
    void isFilterable(bool);
    void isViewChangeable(bool);
    void currentViewTypeChanged(DB::Category::ViewType);
    void viewChanged(DB::ImageSearchInfo);
    void imageCount(int);
    void showSearch();

protected:
    bool eventFilter(QObject *, QEvent *) override;
    void activatePage(int pageIndex);

private Q_SLOTS:
    void resetIconViewSearch();
    void itemClicked(const QModelIndex &);
    void adjustTreeViewColumnSize();
    void emitSignals();

private:
    void changeViewTypeForCurrentView(DB::Category::ViewType type);
    Browser::BrowserPage *currentAction() const;
    void switchToViewType(DB::Category::ViewType);
    void setBranchOpen(const QModelIndex &parent, bool open);
    Browser::BreadcrumbList createPath() const;
    void createWidgets();
    void handleResizeEvent(QMouseEvent *);

private:
    static BrowserWidget *s_instance;
    QList<BrowserPage *> m_list;
    int m_current;
    QStackedWidget *m_stack;
    CenteringIconView *m_listView;
    QTreeView *m_treeView;
    QAbstractItemView *m_curView;
    TreeFilter *m_filterProxy;
    Browser::BreadcrumbList m_breadcrumbs;
    QPoint m_resizePressPos;
    static bool s_isResizing;
};
}

#endif /* BROWSER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

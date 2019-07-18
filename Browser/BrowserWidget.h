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

#ifndef BROWSER_H
#define BROWSER_H
#include "BreadcrumbList.h"
#include "CenteringIconView.h"
#include "Settings/SettingsData.h"
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

public slots:
    void back();
    void forward();
    void go();
    void home();
    void reload();
    void slotSmallListView();
    void slotLargeListView();
    void slotSmallIconView();
    void slotLargeIconView();
    void slotLimitToMatch(const QString &);
    void slotInvokeSeleted();
    void scrollKeyPressed(QKeyEvent *);
    void widenToBreadcrumb(const Browser::Breadcrumb &);

signals:
    void canGoBack(bool);
    void canGoForward(bool);
    void showingOverview();
    void pathChanged(const Browser::BreadcrumbList &);
    void isSearchable(bool);
    void isFilterable(bool);
    void isViewChangeable(bool);
    void currentViewTypeChanged(DB::Category::ViewType);
    void viewChanged();
    void imageCount(uint);

protected:
    bool eventFilter(QObject *, QEvent *) override;
    void activatePage(int pageIndex);

private slots:
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

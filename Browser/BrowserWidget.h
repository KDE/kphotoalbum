/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include <QListView>
#include "Settings/SettingsData.h"

class TreeFilter;
class QSortFilterProxyModel;
class QTreeView;
class QListView;
class QStackedWidget;

namespace DB
{
    class ImageSearchInfo;
}

namespace Browser
{
class BrowserIconViewItemFactory;
class FolderAction;
class BrowserItemFactory;
class BrowserAction;

class BrowserWidget :public QWidget {
    Q_OBJECT
    friend class ImageFolderAction;

public:
    BrowserWidget( QWidget* parent );
    void addSearch( DB::ImageSearchInfo& info );
    void addImageView( const QString& context );
    static BrowserWidget* instance();
    void load( const QString& category, const QString& value );
    DB::ImageSearchInfo currentContext();
    void setFocus();
    QString currentCategory() const;
    void addAction( Browser::BrowserAction* );

    void setModel( QAbstractItemModel* );

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
    void slotLimitToMatch( const QString& );
    void slotInvokeSeleted();
    void scrollLine( int direction );
    void scrollPage( int direction );
    void widenToBreadcrumb( const Browser::Breadcrumb& );

signals:
    void canGoBack( bool );
    void canGoForward( bool );
    void showingOverview();
    void pathChanged( const Browser::BreadcrumbList& );
    void isSearchable( bool );
    void isViewChangeable( bool );
    void currentViewTypeChanged( DB::Category::ViewType );
    void viewChanged();
    void imageCount(uint);

    // bool is true if we have "chosen" some category (ie. when it's safe to change "view as list/ view as icons" stuff)
    void browsingInSomeCategory( bool );

protected:
    OVERRIDE bool eventFilter( QObject*, QEvent* );

private slots:
    void resetIconViewSearch();
    void itemClicked( const QModelIndex& );
    void adjustTreeViewColumnSize();
    void emitSignals();

private:
    void changeViewTypeForCurrentView( DB::Category::ViewType type );
    Browser::BrowserAction* currentAction() const;
    void switchToViewType( DB::Category::ViewType );
    void setBranchOpen( const QModelIndex& parent, bool open );
    Browser::BreadcrumbList createPath() const;
    void createWidgets();

private:
    static BrowserWidget* _instance;
    QList<BrowserAction*> _list;
    int _current;
    QStackedWidget* _stack;
    QListView* _listView;
    QTreeView* _treeView;
    QAbstractItemView* _curView;
    TreeFilter* _filterProxy;
    Browser::BreadcrumbList _breadcrumbs;
};

}

#endif /* BROWSER_H */


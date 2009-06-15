/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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

/**
  \namespace Browser
  \brief The component that allows you to narrow down the image set

  The GUI component of the browser is the BrowserWidget. This widget
  has a QTreeView and a CenteringIconView for displaying its content.

  The class CenteringIconView is a simple QListView that is adapted to
  optionally centering its content.

  The BrowserWidget has a list of \ref BrowserPage's, each page represent an
  item that you can go backward or forward to using the back/forward
  buttons in the toolbar.

  <h2>The %BrowserPage class and its subclasses</h2>

  The \ref BrowserPage is responsible for providing an instance of
  QAbstractItemModel for the \ref BrowserWidget's QTreeView and
  QListView. Besides that it has a number of methods telling properties of
  the given page ( e.g. BrowserPage::viewer() tells if the Browser or the
  Viewer should be used, BrowserPage::isSearchable tells if it possible to
  use the search bar on the page etc).

  Three subclasses currently exists for the BrowserPage:
  \li OverviewPage - the page with (People, Places, Show images etc)
  \li CategoryPage - the page showing the items of a given caetegory
  \li ImageViewPage - represent a thumbnail view

  <h2>Searching</h2>
  The content of the Browser can be narrowed using the search bar, for that
  the class \ref TreeFilter implements a QSortFilterProxyModel that the
  \ref BrowserWidget used for filtering and sorting.

  <h2>Breadcrumbs</h2>
  At the bottom of the main window, a list of breadcrumbs exists for
  navigating the path taken in the browser. That list is made up from
  individual breadcrumbs of the steps. The individual breadcrumbs are
  represented using \ref Breadcrumb and a list of breadcrumbs is
  represented using \ref BreadcrumbList.

  <h2>Models</h2>
  The browser has, as mentioned, two views: a tree view and a list view (for
  icon mode). These are populated using instances of QAbstractItemModel
  which are returned from subclasses of \ref BrowserPage.

  The OverviewPage subclasses QAbstractListModel, and implement the model
  internally. The \ref CategoryPage on the other is slightly more
  complicated, as it either needs to use a simply model for the icon
  view, or a more complicated one for the tree views.

  For that purpose two classes exists: \ref FlatCategoryModel (the simple
  one) and \ref TreeCategoryModel (the tree version). Both classes inherits
  from \ref AbstractCategoryModel, which has some common code for the two.

**/

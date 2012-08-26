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

#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <QLabel>
#include "enums.h"
#include "Settings/SettingsData.h"
#include <q3listview.h>
#include "DB/CategoryPtr.h"

class Q3ListViewItem;
class CategoryItem;
class QToolButton;
class QEvent;
class QRadioButton;
class QLabel;

namespace DB { class ImageInfo; }
namespace CategoryListView { class DragableListView; }
namespace CategoryListView { class CheckDropItem; }

namespace AnnotationDialog
{
using Utilities::StringSet;

class CompletableLineEdit;

class ListSelect :public QWidget {
    Q_OBJECT

public:
    ListSelect( const DB::CategoryPtr& category, QWidget* parent );
    QString category() const;
    QString text() const;
    void setText( const QString& );
    void setSelection( const StringSet& on, const StringSet& partiallyOn = StringSet() );
    StringSet itemsOn() const;
    StringSet itemsOff() const;
    StringSet itemsUnchanged() const;

    bool isAND() const;
    void setMode( UsageMode );

    void populate();

    void showOnlyItemsMatching( const QString& text );
    QWidget* lineEdit();


public slots:
    void slotReturn();
    void slotSortDate();
    void slotSortAlphaTree();
    void slotSortAlphaFlat();
    void toggleSortType();
    void updateListview();
    void rePopulate();

protected slots:
    void itemSelected( Q3ListViewItem* );
    void showContextMenu( Q3ListViewItem*, const QPoint& );
    void setViewSortType( Settings::ViewSortType );
    void limitToSelection();
    void showAllChildren();
    void updateSelectionCount();

protected:
    virtual bool eventFilter( QObject* object, QEvent* event );
    void addItems( DB::CategoryItem* item, Q3ListViewItem* parent );
    void populateAlphaTree();
    void populateAlphaFlat();
    void populateMRU();
    void configureItem( CategoryListView::CheckDropItem* item );
    bool isInputMode() const;
    StringSet itemsOfState( Q3CheckListItem::ToggleState state ) const;
    void checkItem( const QString itemText, bool );
    void ensureAllInstancesAreStateChanged( Q3ListViewItem* item );

private:
    DB::CategoryPtr _category;
    CompletableLineEdit* _lineEdit;
    CategoryListView::DragableListView* _listView;
    QRadioButton* _or;
    QRadioButton* _and;
    UsageMode _mode;
    QToolButton* _alphaTreeSort;
    QToolButton* _alphaFlatSort;
    QToolButton* _dateSort;
    QToolButton* _showSelectedOnly;
    QString _baseTitle;
};

}

#endif /* LISTSELECT_H */


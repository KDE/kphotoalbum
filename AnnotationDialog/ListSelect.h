/* Copyright (C) 2003-2015 Jesper K. Pedersen <blackie@kde.org>

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
#include "DB/CategoryPtr.h"
#include <QList>
#include <config-kpa-kface.h>
#ifdef HAVE_KFACE
#include "FaceManagement/Recognizer.h"
#endif

class QTreeWidgetItem;
class CategoryItem;
class QToolButton;
class QEvent;
class QRadioButton;
class QLabel;

namespace DB { class ImageInfo; }
namespace CategoryListView { class DragableTreeWidget; }
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
    QWidget* lineEdit() const;
    void setPositionable(bool positionableState);
    bool positionable() const;
    bool tagIsChecked(QString tag) const;

public slots:
    void slotReturn();
    void slotSortDate();
    void slotSortAlphaTree();
    void slotSortAlphaFlat();
    void toggleSortType();
    void updateListview();
    void rePopulate();
    void ensureTagIsSelected(QString category, QString tag);

signals:
    /**
     * This signal is emitted whenever a positionable tag is (de)selected.
     */
    void positionableTagSelected(const QString category, const QString tag);
    void positionableTagDeselected(const QString category, const QString tag);
    void positionableTagRenamed(const QString category, const QString oldTag, const QString newTag);

protected slots:
    void itemSelected( QTreeWidgetItem* );
    void showContextMenu(const QPoint& );
    void setViewSortType( Settings::ViewSortType );
    void limitToSelection();
    void showAllChildren();
    void updateSelectionCount();

protected:
    void addItems( DB::CategoryItem* item, QTreeWidgetItem* parent );
    void populateAlphaTree();
    void populateAlphaFlat();
    void populateMRU();
    void configureItem( CategoryListView::CheckDropItem* item );
    bool isInputMode() const;
    StringSet itemsOfState( Qt::CheckState state ) const;
    void checkItem( const QString itemText, bool );
    void ensureAllInstancesAreStateChanged( QTreeWidgetItem* item );

private: // Functions
    bool searchForUntaggedImagesTagNeeded();
    void hideUntaggedImagesTag();

private: // Variables
    DB::CategoryPtr m_category;
    CompletableLineEdit* m_lineEdit;
    CategoryListView::DragableTreeWidget* m_treeWidget;
    QRadioButton* m_or;
    QRadioButton* m_and;
    UsageMode m_mode;
    QToolButton* m_alphaTreeSort;
    QToolButton* m_alphaFlatSort;
    QToolButton* m_dateSort;
    QToolButton* m_showSelectedOnly;
    QString m_baseTitle;
    bool m_positionable;
#ifdef HAVE_KFACE
    FaceManagement::Recognizer* m_recognizer;
#endif
};

}

#endif /* LISTSELECT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

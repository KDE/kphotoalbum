/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <qwidget.h>
#include <qstringlist.h>
#include "Settings/SettingsData.h"
#include <qtoolbutton.h>
class QListViewItem;
class QLabel;
class QCheckBox;
class QListView;

namespace DB
{
    class ImageInfo;
}

namespace AnnotationDialog
{
class CompletableLineEdit;

class ListSelect :public QWidget {
    Q_OBJECT

public:
    ListSelect( const QString& category, QWidget* parent,  const char* name = 0 );
    QString category() const;
    QString text() const;
    void setText( const QString& );
    void setSelection( const QStringList& list );
    QStringList selection();
    void setShowMergeCheckbox( bool b );
    bool doMerge() const;
    bool doRemove() const;
    bool isAND() const;

    enum Mode {INPUT, SEARCH};
    void setMode( Mode );

    void populate();
    void rePopulate();

public slots:
    void slotReturn();

protected slots:
    void itemSelected( QListViewItem* );
    void showContextMenu( QListViewItem*, const QPoint& );
    void setViewSortType( Settings::ViewSortType );
    void slotSortDate();
    void slotSortAlpha();
    void checkBoxStateChanged( int state );
    void removeCheckBoxStateChanged( int state );

protected:
    virtual bool eventFilter( QObject* object, QEvent* event );

private:

    QLabel* _label;
    QString _category;
    CompletableLineEdit* _lineEdit;
    QListView* _listBox;
    QCheckBox* _checkBox;
    QCheckBox* _removeCheckBox;
    Mode _mode;
#ifdef TEMPORARILY_REMOVED
    QListBoxItem* _none;
#endif
    QToolButton* _alphaSort;
    QToolButton* _dateSort;
};

}

#endif /* LISTSELECT_H */


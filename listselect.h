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
#include "options.h"
#include <qtoolbutton.h>
class QListBox;
class QLabel;
class QCheckBox;
class CompletableLineEdit;
class ImageInfo;
class QListBoxItem;

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
    bool isAND() const;

    enum Mode {INPUT, SEARCH};
    void setMode( Mode );

    void populate();

public slots:
    void slotReturn();

protected slots:
    void itemSelected( QListBoxItem* );
    void showContextMenu( QListBoxItem*, const QPoint& );
    void setViewSortType( Options::ViewSortType );
    void slotSortDate();
    void slotSortAlpha();

protected:
    virtual bool eventFilter( QObject* object, QEvent* event );

private:
    QLabel* _label;
    QString _category;
    CompletableLineEdit* _lineEdit;
    QListBox* _listBox;
    QCheckBox* _checkBox;
    Mode _mode;
    QListBoxItem* _none;
    QToolButton* _alphaSort;
    QToolButton* _dateSort;
};

#endif /* LISTSELECT_H */


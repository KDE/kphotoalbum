/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef LISTSELECT_H
#define LISTSELECT_H

#include <qwidget.h>
#include <qstringlist.h>
class QListBox;
class QLabel;
class QCheckBox;
class CompletableLineEdit;
class ImageInfo;
class QListBoxItem;

class ListSelect :public QWidget {
    Q_OBJECT

public:
    ListSelect( const QString& optionGroup, QWidget* parent,  const char* name = 0 );
    void setOptionGroup( const QString& group );
    QString optionGroup() const;
    QString text() const;
    void setText( const QString& );
    void setSelection( const QStringList& list );
    QStringList selection();
    void setShowMergeCheckbox( bool b );
    bool merge() const;

    enum Mode {INPUT, SEARCH};
    void setMode( Mode );

    void populate();

public slots:
    void slotReturn();

protected slots:
    void itemSelected( QListBoxItem* );
    void showContextMenu( QListBoxItem*, const QPoint& );

protected:
    virtual bool eventFilter( QObject* object, QEvent* event );

private:
    QLabel* _label;
    QString _optionGroup;
    CompletableLineEdit* _lineEdit;
    QListBox* _listBox;
    QCheckBox* _merge;
    Mode _mode;
    QListBoxItem* _none;
};

#endif /* LISTSELECT_H */


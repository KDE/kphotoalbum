/* Copyright (C) 2007 Jan Kundrát <jkt@gentoo.org>

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
#ifndef EXIFSYNCWIDGET_H
#define EXIFSYNCWIDGET_H

#include <qlistview.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include "Exif/Syncable.h"

namespace Exif{

class SyncWidget : public QHBox {
    Q_OBJECT

public:
    SyncWidget( const QString& title, QWidget* parent, const QValueList<Syncable::Kind>& items, const char* name = 0 );
    QValueList<Syncable::Kind> items() const;
    void updatePreferred( const QValueList<Syncable::Kind>& items );

protected slots:
    void slotMoveSelectedDown();
    void slotMoveSelectedUp();
    void slotHandleDisabling();

private:
    QValueList<Syncable::Kind> _items;
    QListView* _list;
    QPushButton* _upBut;
    QPushButton* _downBut;
};

}

#endif /* EXIFSYNCWIDGET_H */


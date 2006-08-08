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

#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H
#include <kdialogbase.h>
class QLabel;
class QCheckBox;
class QRadioButton;

namespace MainWindow
{

class DeleteDialog :public KDialogBase {
    Q_OBJECT

public:
    DeleteDialog( QWidget* parent, const char* name = 0 );
    int exec( const QStringList& );

protected slots:
    void deleteImages();

private:
    QStringList _list;
    QLabel* _label;
    QCheckBox* _deleteFromDisk;
    QCheckBox* _block;

};

}


#endif /* DELETEDIALOG_H */


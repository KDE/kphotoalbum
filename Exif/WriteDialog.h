/* Copyright (C) 2003-2006 Jan Kundrát <jkt@gentoo.org>

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

#ifndef WRITEDIALOG_H
#define WRITEDIALOG_H
#include <kdialogbase.h>
class QLabel;
class QCheckBox;
class QRadioButton;

namespace Exif
{

class WriteDialog :public KDialogBase {
    Q_OBJECT

public:
    WriteDialog( QWidget* parent, const char* name = 0 );
    int exec( const QStringList& );

protected slots:
    void write();
    void showFileList();
    bool warnAboutChanges();

private:
    QStringList _list;
    QLabel* _title;
    QCheckBox* _date;
    QCheckBox* _orientation;
    QCheckBox* _label;
    QCheckBox* _description;
    QCheckBox* _categories;
};

}

#endif /* WRITEDIALOG_H */


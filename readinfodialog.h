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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef READINFODIALOG_H
#define READINFODIALOG_H
#include <kdialogbase.h>
#include "imageinfolist.h"
class QLabel;
class QCheckBox;
class QRadioButton;

class ReadInfoDialog :public KDialogBase {
    Q_OBJECT

public:
    ReadInfoDialog( QWidget* parent, const char* name = 0 );
    int exec( const ImageInfoList& );

protected slots:
    void readInfo();
private:
    ImageInfoList _list;
    QLabel* _label;
    QCheckBox* _time;
    QCheckBox* _date;
    QCheckBox* _orientation;
    QCheckBox* _description;
    QCheckBox* _force_time;
    QCheckBox* _force_date;
};


#endif /* READINFODIALOG_H */


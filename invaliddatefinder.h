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

#ifndef INVALIDDATEFINDER_H
#define INVALIDDATEFINDER_H

#include <kdialogbase.h>
class QRadioButton;

class InvalidDateFinder :public KDialogBase {
    Q_OBJECT

public:
    InvalidDateFinder( QWidget* parent, const char* name = 0 );

protected slots:
    virtual void slotOk();

private:
    QRadioButton* _dateNotTime;
    QRadioButton* _missingDate;
    QRadioButton* _partialDate;
    QRadioButton* _missingYear;
};


#endif /* INVALIDDATEFINDER_H */


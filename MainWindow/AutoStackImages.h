/* Copyright (C) 2010 Miika Turkia <miika.turkia@gmail.com>

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

#ifndef AUTOSTACKIMAGES_H
#define AUTOSTACKIMAGES_H

#include <KDialog>
#include "DB/Id.h"
#include "DB/MD5Map.h"
#include <qspinbox.h>
#include <qtextedit.h>
#include <qradiobutton.h>

class QCheckBox;

namespace MainWindow
{

class AutoStackImages :public KDialog {
    Q_OBJECT

public:
    AutoStackImages( QWidget* parent, const DB::IdList& list );

protected slots:
    virtual void accept();

private:
    QCheckBox* _matchingMD5;
    QCheckBox* _continuousShooting;
    QRadioButton* _autostackUnstack;
    QRadioButton* _autostackSkip;
    QRadioButton* _autostackDefault;
    QSpinBox* _continuousThreshold;
    const DB::IdList& _list;
    virtual void matchingMD5( DB::IdList &toBeShown );
    virtual void continuousShooting( DB::IdList &toBeShown );
};

}

#endif /* AUTOSTACKIMAGES_H */

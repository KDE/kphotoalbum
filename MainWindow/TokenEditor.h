/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#ifndef TOKENEDITOR_H
#define TOKENEDITOR_H

#include <kdialogbase.h>
#include <qvaluelist.h>
class QCheckBox;

namespace MainWindow
{

class TokenEditor :public KDialogBase {
    Q_OBJECT

public:
    TokenEditor( QWidget* parent, const char* name = 0 );
    virtual void show();

protected slots:
    void selectAll();
    void selectNone();
    virtual void slotOk();

protected:
    QStringList tokensInUse();

private:
    QValueList<QCheckBox*> _cbs;
};

}

#endif /* TOKENEDITOR_H */


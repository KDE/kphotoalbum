/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H
#include <KDialog>
#include <QLabel>
#include <kjob.h>
#include <qradiobutton.h>
#include "DB/IdList.h"

class QLabel;
class QCheckBox;
class KJob;
namespace KIO {class Job; }
namespace MainWindow
{

class DeleteDialog :public KDialog {
    Q_OBJECT

public:
    DeleteDialog( QWidget* parent );
    int exec(const DB::IdList& list);

protected slots:
    void deleteImages();
    void slotKIOJobCompleted( KJob* );

private:
    DB::IdList _list;
    QLabel* _label;
    QRadioButton* _deleteFile;
    QRadioButton* _useTrash;
    QRadioButton* _deleteFromDb;

};

}


#endif /* DELETEDIALOG_H */


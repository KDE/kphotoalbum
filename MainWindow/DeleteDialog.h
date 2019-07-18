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
#include <DB/FileNameList.h>
#include <QDialog>
#include <QLabel>
#include <kjob.h>
#include <qradiobutton.h>

class QLabel;
class QCheckBox;
class KJob;
namespace KIO
{
class Job;
}
namespace MainWindow
{

class DeleteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteDialog(QWidget *parent);
    // prevent hiding of base class method:
    using QDialog::exec;
    int exec(const DB::FileNameList &list);

protected slots:
    void deleteImages();

private:
    DB::FileNameList m_list;
    QLabel *m_label;
    QRadioButton *m_deleteFile;
    QRadioButton *m_useTrash;
    QRadioButton *m_deleteFromDb;
};
}

#endif /* DELETEDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

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

#ifndef REREADDIALOG_H
#define REREADDIALOG_H

#include <DB/FileNameList.h>

#include <QDialog>

class QCheckBox;
class QLabel;
class QListWidget;

namespace Exif
{

class ReReadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReReadDialog(QWidget *parent);
    // prevent hiding of base class method:
    using QDialog::exec;
    int exec(const DB::FileNameList &);

protected slots:
    void readInfo();
    void warnAboutDates(bool);

private:
    DB::FileNameList m_list;
    QCheckBox *m_exifDB;
    QCheckBox *m_date;
    QCheckBox *m_orientation;
    QCheckBox *m_description;
    QCheckBox *m_force_date;
    QListWidget *m_fileList;
};
}

#endif /* REREADDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

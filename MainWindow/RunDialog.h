/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>
   Copyright (C) 2009-2010 Wes Hardaker <kpa@capturedonearth.com>

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

#ifndef RUNDIALOG_H
#define RUNDIALOG_H

#include <DB/FileNameList.h>

#include <QDialog>
#include <QLineEdit>

namespace MainWindow
{

class RunDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunDialog(QWidget *parent);
    void setImageList(const DB::FileNameList &fileList);
    void show();

protected slots:
    void slotMarkGo();

private:
    bool *m_ok;
    QLineEdit *m_cmd;
    DB::FileNameList m_fileList;
};
}

#endif /* RUNDIALOG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

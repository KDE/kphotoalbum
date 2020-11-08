/* SPDX-FileCopyrightText: 2003-2006 Jesper K. Pedersen <blackie@kde.org>
   SPDX-FileCopyrightText: 2009-2010 Wes Hardaker <kpa@capturedonearth.com>

   SPDX-License-Identifier: GPL-2.0-or-later
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

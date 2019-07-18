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

#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>
#include <qdialog.h>
class QLineEdit;

namespace MainWindow
{

class WelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WelcomeDialog(QWidget *parent = nullptr);
    QString configFileName() const;

protected slots:
    void slotLoadDemo();
    void createSetup();
    void checkFeatures();

private:
    QString m_configFile;
};

class FileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FileDialog(QWidget *parent);
    QString getFileName();
protected slots:
    void slotBrowseForDirecory();

private:
    QLineEdit *m_lineEdit;
};

}

#endif // WELCOMEDIALOG_H
// vi:expandtab:tabstop=4 shiftwidth=4:

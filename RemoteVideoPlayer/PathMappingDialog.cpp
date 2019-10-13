/* Copyright (C) 2003-2019 Jesper K. Pedersen <blackie@kde.org>

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

#include "PathMappingDialog.h"
#include "ui_PathMappingDialog.h"

#include <QFileDialog>

PathMappingDialog::PathMappingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PathMappingDialog)
{
    ui->setupUi(this);
    connect(ui->edit, &QToolButton::clicked, this, &PathMappingDialog::configureHostPath);
}

PathMappingDialog::~PathMappingDialog()
{
    delete ui;
}

QString PathMappingDialog::linuxPath() const
{
    return ui->linuxPath->text();
}

QString PathMappingDialog::hostPath() const
{
    return ui->hostPath->text();
}

void PathMappingDialog::setLinuxPath(const QString &path)
{
    ui->linuxPath->setText(path);
}

void PathMappingDialog::setHostPath(const QString &path)
{
    ui->hostPath->setText(path);
}

void PathMappingDialog::configureHostPath()
{
    const QString path = QFileDialog::getExistingDirectory(this, {}, ui->hostPath->text());
    if (!path.isNull())
        ui->hostPath->setText(path);
}

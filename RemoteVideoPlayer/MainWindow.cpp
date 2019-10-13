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

#include "MainWindow.h"
#include "PathMapper.h"
#include "PathMappingDialog.h"
#include "ui_MainWindow.h"
#include <QDebug>
#include <QLabel>
#include <QToolButton>

MainWindow *MainWindow::m_instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_instance = this;
    m_executor = new Executor(this);
    connect(ui->quit, &QPushButton::clicked, qApp, &QCoreApplication::quit);
    connect(&PathMapper::instance(), &PathMapper::setupChanged, this, &MainWindow::populateMappings);
    populateMappings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addMessage(const QString &message)
{
    m_instance->ui->output->appendPlainText(message);
}

void MainWindow::populateMappings()
{
    qDeleteAll(ui->pathMappings->children());

    auto layout = new QGridLayout(ui->pathMappings);
    int row = -1;

    for (const PathMapper::Mapping &mapping : PathMapper::instance().mappings()) {
        layout->addWidget(new QLabel("<b>Linux Path:</b>"), ++row, 0);
        layout->addWidget(new QLabel(mapping.linuxPath), row, 1);
        layout->addWidget(new QLabel("<b>Host Path:</b>"), row, 2);
        layout->addWidget(new QLabel(mapping.hostPath), row, 3);

        auto edit = new QToolButton;
        edit->setText("Edit");
        connect(edit, &QToolButton::clicked, [linuxPath = mapping.linuxPath, hostPath = mapping.hostPath] { PathMapper::instance().configurePath(linuxPath, hostPath); });
        layout->addWidget(edit, row, 4);

        auto remove = new QToolButton;
        remove->setText("Remove");
        connect(remove, &QToolButton::clicked, [linuxPath = mapping.linuxPath, hostPath = mapping.hostPath] { PathMapper::instance().removeMapping(linuxPath, hostPath); });
        layout->addWidget(remove, row, 5);

        layout->setColumnStretch(6, 1);
    }
}

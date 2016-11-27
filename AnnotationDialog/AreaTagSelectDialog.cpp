/* Copyright (C) 2016 Tobias Leupold <tobias.leupold@web.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Qt includes
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include <QApplication>

// Local includes
#include "AreaTagSelectDialog.h"
#include "CompletableLineEdit.h"
#include "AreaTagSelectLineEdit.h"

AnnotationDialog::AreaTagSelectDialog::AreaTagSelectDialog(CompletableLineEdit* categoryLineEdit,
                                                           QPixmap& areaImage)
    : QDialog(), m_categoryLineEdit(categoryLineEdit)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
    setModal(true);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    QLabel* areaImageLabel = new QLabel();
    areaImageLabel->setPixmap(areaImage);
    mainLayout->addWidget(areaImageLabel);

    AreaTagSelectLineEdit* tagSelect = new AreaTagSelectLineEdit();
    mainLayout->addWidget(tagSelect);
    connect(tagSelect, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(tagSelectKeyPressed(QKeyEvent*)));
}

void AnnotationDialog::AreaTagSelectDialog::tagSelectKeyPressed(QKeyEvent* event)
{
    qDebug() << "I'd like to send my keypresses" << event << "to" << m_categoryLineEdit << "now.";
}

// vi:expandtab:tabstop=4 shiftwidth=4:

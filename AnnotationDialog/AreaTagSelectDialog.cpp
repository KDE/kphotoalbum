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
#include <QPainter>
#include <QKeyEvent>

// Local includes
#include "AreaTagSelectDialog.h"
#include "CompletableLineEdit.h"
#include "AreaTagSelectLineEdit.h"
#include "ResizableFrame.h"

AnnotationDialog::AreaTagSelectDialog::AreaTagSelectDialog(ResizableFrame* area,
                                                           CompletableLineEdit* categoryLineEdit,
                                                           QPixmap& areaImage)
    : QDialog(), m_categoryLineEdit(categoryLineEdit)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    QLabel* areaImageLabel = new QLabel();
    areaImageLabel->setPixmap(areaImage);
    mainLayout->addWidget(areaImageLabel);

    AreaTagSelectLineEdit* tagSelect = new AreaTagSelectLineEdit(area, categoryLineEdit);
    mainLayout->addWidget(tagSelect);
}

void AnnotationDialog::AreaTagSelectDialog::paintEvent(QPaintEvent*)
{
    QColor backgroundColor = Qt::white;
    backgroundColor.setAlpha(160);
    QPainter painter(this);
    painter.fillRect(rect(), backgroundColor);
}

void AnnotationDialog::AreaTagSelectDialog::reject()
{
    // Clear the CompletableLineEdit.
    // Simply doing setText(QString()) does only clear the input, but not update the TreeWidget
    QKeyEvent* backspace = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    while (m_categoryLineEdit->text() != QString()) {
        m_categoryLineEdit->keyPressEvent(backspace);
    }
    QDialog::reject();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

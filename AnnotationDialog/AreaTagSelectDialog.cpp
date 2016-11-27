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

// Local includes
#include "AreaTagSelectDialog.h"

#include "CompletableLineEdit.h"
#include "ListSelect.h"
#include "ResizableFrame.h"


// Qt includes
#include <QApplication>
#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>

AnnotationDialog::AreaTagSelectDialog::AreaTagSelectDialog(AnnotationDialog::ResizableFrame *area, ListSelect *ls, QPixmap &areaImage)
    :QDialog()
    , m_category(ls->category())
    , m_area(area)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    QLabel* areaImageLabel = new QLabel();
    areaImageLabel->setPixmap(areaImage);
    mainLayout->addWidget(areaImageLabel);

    CompletableLineEdit* tagSelect = new CompletableLineEdit(ls, this);
    ls->connectLineEdit(tagSelect);
    connect(tagSelect, &KLineEdit::returnPressed, this, &AreaTagSelectDialog::slotSetTag);
    mainLayout->addWidget(tagSelect);

}

void AnnotationDialog::AreaTagSelectDialog::slotSetTag(const QString &tag)
{
    QString enteredText = tag.trimmed();
    if (!enteredText.isEmpty())
    {
        m_area->setTagData(m_category, enteredText);
    }
    this->accept();
}

void AnnotationDialog::AreaTagSelectDialog::paintEvent(QPaintEvent*)
{
    QColor backgroundColor = Qt::white;
    backgroundColor.setAlpha(160);
    QPainter painter(this);
    painter.fillRect(rect(), backgroundColor);
}

// vi:expandtab:tabstop=4 shiftwidth=4:

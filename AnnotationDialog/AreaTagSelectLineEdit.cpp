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
#include <QKeyEvent>
#include <QDebug>
#include <QDialog>

// Local includes
#include "AreaTagSelectLineEdit.h"
#include "CompletableLineEdit.h"
#include "ResizableFrame.h"

AnnotationDialog::AreaTagSelectLineEdit::AreaTagSelectLineEdit(ResizableFrame* area,
                                                               CompletableLineEdit* categoryLineEdit)
: QLineEdit(), m_categoryLineEdit(categoryLineEdit), m_area(area)
{
}

void AnnotationDialog::AreaTagSelectLineEdit::keyPressEvent(QKeyEvent *event)
{
    QString enteredTag;
    if (event->key() == Qt::Key_Return) {
        enteredTag = text();
    }

    m_categoryLineEdit->keyPressEvent(event);
    setText(m_categoryLineEdit->text());
    setCursorPosition(m_categoryLineEdit->cursorPosition());

    if (m_categoryLineEdit->hasSelectedText()) {
        setSelection(m_categoryLineEdit->selectionStart(),
                     m_categoryLineEdit->selectedText().length());
    }

    if (! enteredTag.isEmpty()) {
        dynamic_cast<QDialog*>(parent())->accept();
        m_area->setTagData(m_categoryLineEdit->objectName(), enteredTag);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

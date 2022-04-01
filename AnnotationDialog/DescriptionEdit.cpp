// SPDX-FileCopyrightText: 2014-2018 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#include "DescriptionEdit.h"

#include <QKeyEvent>

AnnotationDialog::DescriptionEdit::DescriptionEdit(QWidget *parent)
    : KTextEdit(parent)
{
}

AnnotationDialog::DescriptionEdit::~DescriptionEdit()
{
}

void AnnotationDialog::DescriptionEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
        emit pageUpDownPressed(event);
    } else {
        QTextEdit::keyPressEvent(event);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

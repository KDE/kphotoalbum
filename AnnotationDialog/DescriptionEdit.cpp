// SPDX-FileCopyrightText: 2014-2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
        Q_EMIT pageUpDownPressed(event);
    } else {
        QTextEdit::keyPressEvent(event);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DescriptionEdit.cpp"

// SPDX-FileCopyrightText: 2014 - 2022 Tobias Leupold <tl at stonemx dot de>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "DescriptionEdit.h"

#include <KLocalizedString>
#include <QKeyEvent>

AnnotationDialog::DescriptionEdit::DescriptionEdit(QWidget *parent)
    : KTextEdit(parent)
{
    setProperty("WantsFocus", true);
    setObjectName(i18n("Description"));
    setCheckSpellingEnabled(true);
    setTabChangesFocus(true); // this allows tabbing to the next item in the tab order.
}

AnnotationDialog::DescriptionEdit::~DescriptionEdit()
{
}

QString AnnotationDialog::DescriptionEdit::description() const
{
    return toPlainText();
}

void AnnotationDialog::DescriptionEdit::setDescription(const QString &text)
{
    m_originalText = text;
    setConflictWarning(QString());
    KTextEdit::setPlainText(text);
}

bool AnnotationDialog::DescriptionEdit::changed() const
{
    return (m_originalText != toPlainText());
}

bool AnnotationDialog::DescriptionEdit::isEmpty() const
{
    return toPlainText().isEmpty();
}

void AnnotationDialog::DescriptionEdit::setConflictWarning(const QString &placeholderText)
{
    clear();
    m_originalText.clear();
    KTextEdit::setPlaceholderText(placeholderText);
}

bool AnnotationDialog::DescriptionEdit::hasConflictWarning() const
{
    return (!placeholderText().isEmpty());
}

void AnnotationDialog::DescriptionEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown) {
        Q_EMIT pageUpDownPressed(event);
    } else {
        KTextEdit::keyPressEvent(event);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_DescriptionEdit.cpp"

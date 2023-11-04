// SPDX-FileCopyrightText: 2014 - 2022 Tobias Leupold <tl at stonemx dot de>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef DESCRIPTIONEDIT_H
#define DESCRIPTIONEDIT_H

#include <KTextEdit>

class QKeyEvent;

namespace AnnotationDialog
{

/**
 * @brief The DescriptionEdit class improves upon the KTextEdit, adding a changed() method and an intuitive API to the case
 * when several images have a conflicting description.
 *
 * \note Don't use the base class methods like QTextEdit::setPlainText() and QTextEdit::setPlaceholderText() to manipulate the
 * DescriptionEdit. We can't override these methods since they are not virtual, and private inheritance won't work here because
 * DescriptionEdit needs to be a QWidget.
 *
 */
class DescriptionEdit : public KTextEdit
{
    Q_OBJECT

public:
    explicit DescriptionEdit(QWidget *parent = nullptr);
    ~DescriptionEdit() override;

    /**
     * @brief description
     * @return QTextEdit::toPlainText()
     */
    QString description() const;

    /**
     * @brief setDescription sets the text editor's contents as plain text and sets the comparison QString for changed().
     * This action also removes the ConflictWarning text if set.
     * @param text
     */
    void setDescription(const QString &text);

    /**
     * @brief changed
     * @return \c true, if the text that was set with setDescription was changed. \c false otherwise.
     */
    bool changed() const;

    /**
     * @brief isEmpty is shorthand for description().isEmpty()
     * @return
     */
    bool isEmpty() const;

    /**
     * @brief setConflictWarning sets the text editor's placeholder text to the given QString and marks the editor to have a placeholder state.
     * This action also clears the editor's content.
     * @param placeholderText
     */
    void setConflictWarning(const QString &placeholderText);

    /**
     * @brief hasConflictWarning
     * @return \true, if there is currently a ConflictWarning text set for the DescriptionEdit
     */
    bool hasConflictWarning() const;

Q_SIGNALS:
    void pageUpDownPressed(QKeyEvent *event);

private:
    void keyPressEvent(QKeyEvent *event) override;

    QString m_originalText;
};

}

#endif // DESCRIPTIONEDIT_H

// vi:expandtab:tabstop=4 shiftwidth=4:

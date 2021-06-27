// SPDX-FileCopyrightText: 2003-2019 The KPhotoAlbum Development Team
// SPDX-FileCopyrightText: 2020 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CompletableLineEdit.h"

#include "ListSelect.h"
#include "ResizableFrame.h"

#include <QKeyEvent>
#include <QRegExp>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

AnnotationDialog::CompletableLineEdit::CompletableLineEdit(ListSelect *parent)
    : KLineEdit(parent)
    , m_listSelect(parent)
{
}

AnnotationDialog::CompletableLineEdit::CompletableLineEdit(AnnotationDialog::ListSelect *ls, QWidget *parent)
    : KLineEdit(parent)
    , m_listSelect(ls)
{
}

void AnnotationDialog::CompletableLineEdit::setListView(QTreeWidget *listView)
{
    m_listView = listView;
}

void AnnotationDialog::CompletableLineEdit::setMode(UsageMode mode)
{
    m_mode = mode;
}

void AnnotationDialog::CompletableLineEdit::keyPressEvent(QKeyEvent *ev)
{
    if (ev->key() == Qt::Key_Down || ev->key() == Qt::Key_Up) {
        selectPrevNextMatch(ev->key() == Qt::Key_Down);
        return;
    }

    if (m_mode == SearchMode && (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter)) { //Confirm autocomplete, deselect all text
        handleSpecialKeysInSearch(ev);
        m_listSelect->showOnlyItemsMatching(QString()); // Show all again after confirming autocomplete suggestion.
        return;
    }

    if (m_mode != SearchMode && isSpecialKey(ev))
        return; // Don't insert the special character.

    if (ev->key() == Qt::Key_Space && ev->modifiers() & Qt::ControlModifier) {
        mergePreviousImageSelection();
        return;
    }

    QString prevContent = text();

    if (ev->text().isEmpty() || !ev->text()[0].isPrint()) {
        // If final Return is handled by the default implementation,
        // it can "leak" to other widgets. So we swallow it here:
        if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter)
            emit KLineEdit::returnKeyPressed(text());
        else
            KLineEdit::keyPressEvent(ev);
        if (prevContent != text())
            m_listSelect->showOnlyItemsMatching(text());
        return;
    }

    // &,|, or ! should result in the current item being inserted
    if (m_mode == SearchMode && isSpecialKey(ev)) {
        handleSpecialKeysInSearch(ev);
        m_listSelect->showOnlyItemsMatching(QString()); // Show all again after a special caracter.
        return;
    }

    int cursorPos = cursorPosition();
    int selStart = selectionStart();

    KLineEdit::keyPressEvent(ev);

    // Find the text of the current item
    int itemStart = 0;
    QString input = text();
    if (m_mode == SearchMode) {
        input = input.left(cursorPosition());
        itemStart = input.lastIndexOf(QRegExp(QString::fromLatin1("[!&|]"))) + 1;

        if (itemStart > 0) {
            itemStart++;
        }

        input = input.mid(itemStart);
    }

    // Find the text in the listView
    QTreeWidgetItem *item = findItemInListView(input);
    if (!item && m_mode == SearchMode) {
        // revert
        setText(prevContent);
        setCursorPosition(cursorPos);
        item = findItemInListView(input);
        if (selStart >= 0)
            setSelection(selStart, prevContent.length()); // Reset previous selection.
    }

    if (item) {
        selectItemAndUpdateLineEdit(item, itemStart, input);
        m_listSelect->showOnlyItemsMatching(input);
    } else if (m_mode != SearchMode)
        m_listSelect->showOnlyItemsMatching(input);
}

/**
 * Search for the first item in the appearance order, which matches text.
 */
QTreeWidgetItem *AnnotationDialog::CompletableLineEdit::findItemInListView(const QString &text)
{
    for (QTreeWidgetItemIterator itemIt(m_listView); *itemIt; ++itemIt) {
        // Hide the "untagged image" tag from the auto-completion
        if ((*itemIt)->isHidden()) {
            continue;
        }

        if (itemMatchesText(*itemIt, text)) {
            return *itemIt;
        }
    }

    return nullptr;
}

bool AnnotationDialog::CompletableLineEdit::itemMatchesText(QTreeWidgetItem *item, const QString &text)
{
    return item->text(0).toLower().startsWith(text.toLower());
}

bool AnnotationDialog::CompletableLineEdit::isSpecialKey(QKeyEvent *ev)
{
    return (ev->text() == QString::fromLatin1("&") || ev->text() == QString::fromLatin1("|") || ev->text() == QString::fromLatin1("!")
            /* || ev->text() == "(" */
    );
}

void AnnotationDialog::CompletableLineEdit::handleSpecialKeysInSearch(QKeyEvent *ev)
{
    int cursorPos = cursorPosition();
    QString txt;
    int additionalLength;

    if (!isSpecialKey(ev)) {
        txt = text().left(cursorPos) + ev->text() + text().mid(cursorPos);
        additionalLength = 0;
    } else {
        txt = text() + QString::fromUtf8(" %1 ").arg(ev->text());
        cursorPos += 2;
        additionalLength = 2;
    }
    setText(txt);

    if (!isSpecialKey(ev)) {
        //Special handling for ENTER to position the cursor correctly
        setText(text().left(text().size() - 1));
        cursorPos--;
    }

    setCursorPosition(cursorPos + ev->text().length() + additionalLength);
    deselect();

    // Select the item in the listView - not perfect but acceptable for now.
    int start = txt.lastIndexOf(QRegExp(QString::fromLatin1("[!&|]")), cursorPosition() - 2) + 1;
    if (start > 0) {
        start++;
    }
    QString input = txt.mid(start, cursorPosition() - start);

    if (!input.isEmpty()) {
        QTreeWidgetItem *item = findItemInListView(input);
        if (item) {
            item->setCheckState(0, Qt::Checked);
        }
    }
}

void AnnotationDialog::CompletableLineEdit::selectPrevNextMatch(bool next)
{
    QTreeWidgetItem *item { nullptr };

    // the current item is usually the selected one...
    QList<QTreeWidgetItem *> selectedItems = m_listView->selectedItems();
    if (!selectedItems.isEmpty())
        item = selectedItems.at(0);

    // ...except when the selected one is filtered out:
    if (!item || item->isHidden()) {
        // in that case, we select the first item in the viewport
        item = m_listView->itemAt(0, 0);
    }
    if (!item)
        return;
    QTreeWidgetItem *baseItem = item;

    // only go to the next item, if there was a "previous" selected item:
    if (!selectedItems.isEmpty()) {
        if (next)
            item = m_listView->itemBelow(item);
        else
            item = m_listView->itemAbove(item);
    }

    // select current item if there is no next/prev item:
    if (!item) {
        item = baseItem;
    }

    // extract last component of line edit
    int itemStart = text().lastIndexOf(QRegExp(QString::fromLatin1("[!&|]"))) + 1;
    selectItemAndUpdateLineEdit(item, itemStart, text().left(selectionStart()));
}

void AnnotationDialog::CompletableLineEdit::selectItemAndUpdateLineEdit(QTreeWidgetItem *item,
                                                                        const int itemStart,
                                                                        const QString &inputText)
{
    m_listView->setCurrentItem(item);
    m_listView->scrollToItem(item);

    QString txt = text().left(itemStart) + item->text(0) + text().mid(cursorPosition());

    setText(txt);
    setSelection(itemStart + inputText.length(), item->text(0).length() - inputText.length());
}

void AnnotationDialog::CompletableLineEdit::mergePreviousImageSelection()
{
}

// vi:expandtab:tabstop=4 shiftwidth=4:

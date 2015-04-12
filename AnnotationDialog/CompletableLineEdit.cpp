/* Copyright (C) 2003-2015 Jesper K. Pedersen <blackie@kde.org>

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
#include "CompletableLineEdit.h"
#include "ListSelect.h"
#include <QRegExp>
#include <QKeyEvent>
#include <QTreeWidgetItemIterator>
#include <QTreeWidgetItem>
#include <QDebug>

AnnotationDialog::CompletableLineEdit::CompletableLineEdit( ListSelect* parent )
    :KLineEdit( parent )
{
    m_listSelect = parent;
}

void AnnotationDialog::CompletableLineEdit::setListView( QTreeWidget* listView )
{
    m_listView = listView;
}

void AnnotationDialog::CompletableLineEdit::setMode( UsageMode mode )
{
    m_mode = mode;
}

void AnnotationDialog::CompletableLineEdit::keyPressEvent( QKeyEvent* ev )
{
    if ( ev->key() == Qt::Key_Down || ev->key() == Qt::Key_Up ) {
        selectPrevNextMatch( ev->key() == Qt::Key_Down );
        return;
    }

    if ( m_mode == SearchMode && ( ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) ) { //Confirm autocomplete, deselect all text
        handleSpecialKeysInSearch( ev );
        m_listSelect->showOnlyItemsMatching( QString() ); // Show all again after confirming autocomplete suggestion.
        return;
    }

    if ( m_mode != SearchMode && isSpecialKey( ev ) )
        return; // Don't insert the special character.

    if ( ev->key() == Qt::Key_Space && ev->modifiers() & Qt::ControlModifier ) {
        mergePreviousImageSelection();
        return;
    }

    QString prevContent = text();

    if ( ev->text().isEmpty() || !ev->text()[0].isPrint() ) {
        // If final Return is handled by the default implementation,
        // it can "leak" to other widgets. So we swallow it here:
        if ( ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter )
            emit KLineEdit::returnPressed( text() );
        else
            KLineEdit::keyPressEvent( ev );
        if ( prevContent != text() )
            m_listSelect->showOnlyItemsMatching( text() );
        return;
    }

    // &,|, or ! should result in the current item being inserted
    if ( m_mode == SearchMode && isSpecialKey( ev ) )  {
        handleSpecialKeysInSearch( ev );
        m_listSelect->showOnlyItemsMatching( QString() ); // Show all again after a special caracter.
        return;
    }

    int cursorPos = cursorPosition();
    int selStart = selectionStart();

    KLineEdit::keyPressEvent( ev );


    // Find the text of the current item
    int itemStart = 0;
    QString input = text();
    if ( m_mode == SearchMode )  {
        input = input.left( cursorPosition() );
        itemStart = input.lastIndexOf(QRegExp(QString::fromLatin1("[!&|]"))) + 1;

        if (itemStart > 0) {
            itemStart++;
        }

        input = input.mid( itemStart );
    }

    // Find the text in the listView
    QTreeWidgetItem* item = findItemInListView( input );
    if ( !item && m_mode == SearchMode )  {
        // revert
        setText( prevContent );
        setCursorPosition( cursorPos );
        item = findItemInListView( input );
        if(selStart>=0)
            setSelection( selStart, prevContent.length() ); // Reset previous selection.
    }

    if ( item ) {
        selectItemAndUpdateLineEdit( item, itemStart, input );
        m_listSelect->showOnlyItemsMatching( input );
    }
    else if (m_mode != SearchMode )
        m_listSelect->showOnlyItemsMatching( input );
}

/**
 * Search for the first item in the appearance order, which matches text.
 */
QTreeWidgetItem* AnnotationDialog::CompletableLineEdit::findItemInListView(const QString& text)
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

bool AnnotationDialog::CompletableLineEdit::itemMatchesText(QTreeWidgetItem *item, const QString& text )
{
    return item->text(0).toLower().startsWith( text.toLower() );
}

bool AnnotationDialog::CompletableLineEdit::isSpecialKey( QKeyEvent* ev )
{
    return ( ev->text() == QString::fromLatin1("&") ||
             ev->text() == QString::fromLatin1("|") ||
             ev->text() == QString::fromLatin1("!")
             /* || ev->text() == "(" */
        );
}

void AnnotationDialog::CompletableLineEdit::handleSpecialKeysInSearch( QKeyEvent* ev )
{
    int cursorPos = cursorPosition();
    QString txt;
    int additionalLength;

    if (! isSpecialKey(ev)) {
        txt = text().left(cursorPos) + ev->text() + text().mid(cursorPos);
        additionalLength = 0;
    } else {
        txt = text() + QString::fromUtf8(" %1 ").arg(ev->text());
        cursorPos += 2;
        additionalLength = 2;
    }
    setText(txt);

    if (! isSpecialKey(ev)) {
        //Special handling for ENTER to position the cursor correctly
        setText(text().left(text().size() - 1));
        cursorPos--;
    }

    setCursorPosition( cursorPos + ev->text().length() + additionalLength);
    deselect();

    // Select the item in the listView - not perfect but acceptable for now.
    int start = txt.lastIndexOf(QRegExp(QString::fromLatin1("[!&|]")), cursorPosition() - 2) + 1;
    if (start > 0) {
        start++;
    }
    QString input = txt.mid(start, cursorPosition() - start);

    if (! input.isEmpty()) {
        QTreeWidgetItem* item = findItemInListView(input);
        if (item) {
            item->setCheckState(0, Qt::Checked);
        }
    }
}

void AnnotationDialog::CompletableLineEdit::selectPrevNextMatch( bool next )
{
    int itemStart = text().lastIndexOf( QRegExp(QString::fromLatin1("[!&|]")) ) +1;
    QString input = text().mid( itemStart );

    QList<QTreeWidgetItem*> items = m_listView->findItems( input, Qt::MatchContains, 0 );
    if ( items.isEmpty() )
        return;
    QTreeWidgetItem* item = items.at(0);

    if ( next )
        item = m_listView->itemBelow(item);
    else
        item = m_listView->itemAbove(item);

    if ( item )
        selectItemAndUpdateLineEdit( item, itemStart, text().left( selectionStart() ) );
}

void AnnotationDialog::CompletableLineEdit::selectItemAndUpdateLineEdit(QTreeWidgetItem* item,
                                                                        const int itemStart,
                                                                        const QString& inputText)
{
    m_listView->setCurrentItem( item );
    m_listView->scrollToItem( item );

    QString txt = text().left(itemStart) + item->text(0) + text().mid( cursorPosition() );

    setText( txt );
    setSelection( itemStart + inputText.length(), item->text(0).length() - inputText.length() );
}

void AnnotationDialog::CompletableLineEdit::mergePreviousImageSelection()
{

}


// vi:expandtab:tabstop=4 shiftwidth=4:

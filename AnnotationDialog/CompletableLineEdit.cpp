/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

AnnotationDialog::CompletableLineEdit::CompletableLineEdit( ListSelect* parent )
    :QLineEdit( parent )
{
    _listSelect = parent;
}

void AnnotationDialog::CompletableLineEdit::setListView( Q3ListView* listView )
{
    _listView = listView;
}

void AnnotationDialog::CompletableLineEdit::setMode( UsageMode mode )
{
    _mode = mode;
}

void AnnotationDialog::CompletableLineEdit::keyPressEvent( QKeyEvent* ev )
{
    if ( ev->key() == Qt::Key_Down || ev->key() == Qt::Key_Up ) {
        selectPrevNextMatch( ev->key() == Qt::Key_Down );
        return;
    }

    if ( _mode == SearchMode && ( ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) ) { //Confirm autocomplete, deselect all text
        handleSpecialKeysInSearch( ev );
        _listSelect->showOnlyItemsMatching( QString() ); // Show all again after confirming autocomplete suggestion.
        return;
    }

    if ( _mode != SearchMode && isSpecialKey( ev ) )
        return; // Don't insert the special character.

    if ( ev->key() == Qt::Key_Space && ev->modifiers() & Qt::ControlModifier ) {
        mergePreviousImageSelection();
        return;
    }

    QString prevContent = text();

    if ( ev->text().isEmpty() || !ev->text()[0].isPrint() ) {
        QLineEdit::keyPressEvent( ev );
        if ( prevContent != text() )
            _listSelect->showOnlyItemsMatching( text() );
        return;
    }

    // &,|, or ! should result in the current item being inserted
    if ( _mode == SearchMode && isSpecialKey( ev ) )  {
        handleSpecialKeysInSearch( ev );
        _listSelect->showOnlyItemsMatching( QString() ); // Show all again after a special caracter.
        return;
    }

    int cursorPos = cursorPosition();
    int selStart = selectionStart();

    QLineEdit::keyPressEvent( ev );


    // Find the text of the current item
    int itemStart = 0;
    QString input = text();
    if ( _mode == SearchMode )  {
        input = input.left( cursorPosition() );
        itemStart = input.lastIndexOf( QRegExp(QString::fromLatin1("[!&|]")) ) +1;
        input = input.mid( itemStart );
    }

    // Find the text in the listView
    Q3ListViewItem* item = findItemInListView( input );
    if ( !item && _mode == SearchMode )  {
        // revert
        setText( prevContent );
        setCursorPosition( cursorPos );
        item = findItemInListView( input );
        if(selStart>=0)
            setSelection( selStart, prevContent.length() ); // Reset previous selection.
    }

    if ( item )	{
        selectItemAndUpdateLineEdit( item, itemStart, input );
        _listSelect->showOnlyItemsMatching( input );
    }
    else if (_mode != SearchMode )
        _listSelect->showOnlyItemsMatching( input );
}

/**
 * Search for the first item in the appearance order, which matches text.
 */
Q3ListViewItem* AnnotationDialog::CompletableLineEdit::findItemInListView( const QString& text )
{
    for ( Q3ListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt ) {
        if ( itemMatchesText( *itemIt, text ) )
            return *itemIt;
    }
    return 0;
}

bool AnnotationDialog::CompletableLineEdit::itemMatchesText( Q3ListViewItem* item, const QString& text )
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

    QString txt = text().left(cursorPos) + ev->text() + text().mid( cursorPos );
    setText( txt );
    if(!isSpecialKey(ev) ) cursorPos--; //Special handling for ENTER to position the cursor correctly
    setCursorPosition( cursorPos + ev->text().length() );
    deselect();

    // Select the item in the listView - not perfect but acceptable for now.
    int start = txt.lastIndexOf( QRegExp(QString::fromLatin1("[!&|]")), cursorPosition() -2 ) +1;
    QString input = txt.mid( start, cursorPosition()-start-1 );

    if ( !input.isEmpty() ) {
        Q3ListViewItem* item = findItemInListView( input );
        if ( item )
            item->setSelected( true );
    }
}

void AnnotationDialog::CompletableLineEdit::selectPrevNextMatch( bool next )
{
    int itemStart = text().lastIndexOf( QRegExp(QString::fromLatin1("[!&|]")) ) +1;
    QString input = text().mid( itemStart );

    Q3ListViewItem* item = _listView->findItem( input, 0 );
    if ( !item )
        return;

    if ( next )
        item = item->itemBelow();
    else
        item = item->itemAbove();

    if ( item )
        selectItemAndUpdateLineEdit( item, itemStart, text().left( selectionStart() ) );
}

void AnnotationDialog::CompletableLineEdit::selectItemAndUpdateLineEdit( Q3ListViewItem* item,
                                                                         const int itemStart, const QString& inputText )
{
    _listView->setCurrentItem( item );
    _listView->ensureItemVisible( item );

    QString txt = text().left(itemStart) + item->text(0) + text().mid( cursorPosition() );

    setText( txt );
    setSelection( itemStart + inputText.length(), item->text(0).length() - inputText.length() );
}

void AnnotationDialog::CompletableLineEdit::mergePreviousImageSelection()
{

}



/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "listselect.h"
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvalidator.h>
#include "options.h"
#include "imageinfo.h"
#include <qpopupmenu.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

class CompletableLineEdit :public QLineEdit {
public:
    CompletableLineEdit( QWidget* parent,  const char* name = 0 );
    void setListBox( QListBox* );
    void setMode( ListSelect::Mode mode );
protected:
    virtual void keyPressEvent( QKeyEvent* ev );
    QListBoxItem* findItemInListBox( const QString& startWith );

private:
    QListBox* _listbox;
    ListSelect::Mode _mode;
};


CompletableLineEdit::CompletableLineEdit( QWidget* parent, const char* name )
    :QLineEdit( parent, name )
{
}

void CompletableLineEdit::setListBox( QListBox* listbox )
{
    _listbox = listbox;
}

void CompletableLineEdit::setMode( ListSelect::Mode mode )
{
    _mode = mode;
}

// Better hoope this monster works....
void CompletableLineEdit::keyPressEvent( QKeyEvent* ev )
{
    if ( ev->key() == Key_Return )  {
        QLineEdit::keyPressEvent( ev );
        return;
    }

    if ( !ev->text().isEmpty() && ev->text()[0].isPrint() )  {
        bool special = ( ev->text() == QString::fromLatin1("&") || ev->text() == QString::fromLatin1("|") || ev->text() == QString::fromLatin1("!") /* || ev->text() == "(" */ );
        if ( _mode == ListSelect::INPUT && special )  {
            // Don't insert the special character.
            return;
        }

        QString content = text();
        int cursorPos = cursorPosition();

        // &,|, or ! should result in the current item being inserted
        if ( _mode == ListSelect::SEARCH && special )  {
            QString txt = text().left(cursorPos) + ev->text() + text().mid( cursorPos );
            setText( txt );
            setCursorPosition( cursorPos + ev->text().length() );
            deselect();

            // Select the item in the listbox - not perfect but acceptable for now.
            int start = txt.findRev( QRegExp(QString::fromLatin1("[!&|]")), cursorPosition() -2 ) +1;
            QString input = txt.mid( start, cursorPosition()-start-1 );

            if ( !input.isEmpty() ) {
                QListBoxItem* item = findItemInListBox( input );
                if ( item )
                    _listbox->setSelected( item, true );
            }

            return;
        }


        QLineEdit::keyPressEvent( ev );


        // Find the text of the current item
        int start = 0;
        QString input = text();
        if ( _mode == ListSelect::SEARCH )  {
            input = input.left( cursorPosition() );
            start = input.findRev( QRegExp(QString::fromLatin1("[!&|]")) ) +1;
            input = input.mid( start );
        }

        // Find the text in the listbox
        QListBoxItem* item = findItemInListBox( input );
        if ( !item && _mode == ListSelect::SEARCH )  {
            // revert
            setText( content );
            setCursorPosition( cursorPos );
            item = findItemInListBox( input );
        }

        if ( item )  {
            _listbox->setCurrentItem( item );
            _listbox->ensureCurrentVisible();

            QString txt = text().left(start) + item->text() + text().mid( cursorPosition() );
            setText( txt );
            setSelection( start + input.length(), item->text().length() - input.length() );
        }
    }

    else
        QLineEdit::keyPressEvent( ev );
}

// QListBox::findItem does not search for the item in the order they appear
// in the listbox, which is necesary for us here.
QListBoxItem* CompletableLineEdit::findItemInListBox( const QString& text )
{
    for ( QListBoxItem* item = _listbox->firstItem();
          item; item = item->next() ) {
        if ( item->text().lower().startsWith( text.lower() ) )
            return item;
    }
    return 0;
}


ListSelect::ListSelect( const QString& optionGroup, QWidget* parent, const char* name )
    : QWidget( parent,  name ), _optionGroup( optionGroup )
{
    QVBoxLayout* layout = new QVBoxLayout( this,  6 );

    _label = new QLabel( Options::instance()->textForOptionGroup( optionGroup ), this );
    _label->setAlignment( AlignCenter );
    layout->addWidget( _label );

    _lineEdit = new CompletableLineEdit( this );
    layout->addWidget( _lineEdit );

    _listBox = new QListBox( this );
    _listBox->setSelectionMode( QListBox::Multi );
    connect( _listBox, SIGNAL( clicked( QListBoxItem*  ) ),  this,  SLOT( itemSelected( QListBoxItem* ) ) );
    connect( _listBox, SIGNAL( contextMenuRequested( QListBoxItem*, const QPoint& ) ),
             this, SLOT(showContextMenu( QListBoxItem*, const QPoint& ) ) );
    layout->addWidget( _listBox );
    _listBox->installEventFilter( this );

    _merge = new QCheckBox( i18n("Merge"),  this );
    _merge->setChecked( true );
    layout->addWidget( _merge );

    _lineEdit->setListBox( _listBox );
    connect( _lineEdit, SIGNAL( returnPressed() ),  this,  SLOT( slotReturn() ) );

    populate();
}

void ListSelect::setOptionGroup( const QString& optionGroup )
{
    _optionGroup = optionGroup;
}

void ListSelect::slotReturn()
{
    if ( _mode == INPUT )  {
        QString txt = _lineEdit->text();
        if ( txt.isEmpty() )
            return;

        QListBoxItem* item = _listBox->findItem( txt,  ExactMatch );

        if ( !item ) {
            item = new QListBoxText( _listBox, txt );
        }
        Options* options = Options::instance();
        options->addOption( _optionGroup, txt);

        // move item to front
        _listBox->takeItem( item );
        _listBox->insertItem( item, 0 );
        _listBox->setContentsPos( 0,0 );

        _listBox->setSelected( item,  true );
        _lineEdit->clear();
    }
}

QString ListSelect::optionGroup() const
{
    return _optionGroup;
}

void ListSelect::setSelection( const QStringList& list )
{
    _listBox->clearSelection();

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        QListBoxItem* item = _listBox->findItem( *it,  ExactMatch );
        if ( !item )  {
            _listBox->insertItem( *it );
            item = _listBox->findItem( *it,  ExactMatch );
            Options::instance()->addOption( _optionGroup, *it);
        }
        _listBox->setSelected( item,  true );
    }
    _lineEdit->clear();
}

QStringList ListSelect::selection()
{
    QStringList list;
    for ( QListBoxItem* item = _listBox->firstItem(); item; item = item->next() )  {
        if ( item->isSelected() )
            list.append( item->text() );
    }
    return list;
}

void ListSelect::setShowMergeCheckbox( bool b )
{
    _merge->setEnabled( b );
}

bool ListSelect::merge() const
{
    return _merge->isChecked();
}

void ListSelect::setMode( Mode mode )
{
    _mode = mode;
    _lineEdit->setMode( mode );
    if ( mode == SEARCH) {
        QListBoxItem * none = new QListBoxText( 0, i18n("**NONE**") );
        _listBox->insertItem( none, 0 );
    }
}

QWidget* ListSelect::firstTabWidget() const
{
    return _lineEdit;
}

QWidget* ListSelect::lastTabWidget() const
{
    return _lineEdit;
}

QString ListSelect::text() const
{
    return _lineEdit->text();
}

void ListSelect::setText( const QString& text )
{
    _lineEdit->setText( text );
    _listBox->clearSelection();
}

void ListSelect::itemSelected( QListBoxItem* item )
{
    if ( _mode == SEARCH )  {
        QString txt = item->text();
        QString res;
        QRegExp regEnd( QString::fromLatin1("\\s*[&|!]\\s*$") );
        QRegExp regStart( QString::fromLatin1("^\\s*[&|!]\\s*") );
        if ( item->isSelected() )  {
            int index = _lineEdit->cursorPosition();
            QString start = _lineEdit->text().left(index);
            QString end =  _lineEdit->text().mid(index);

            res = start;
            if ( !start.isEmpty() && !start.contains( regEnd ) )
                 res += QString::fromLatin1("&");
            res += txt;
            if ( !end.isEmpty() && !end.contains( regStart ) )
                res += QString::fromLatin1("&");
            res += end;
        }
        else {
            int index = _lineEdit->text().find( txt );
            if ( index == -1 )
                return;

            QString start = _lineEdit->text().left(index);
            QString end =  _lineEdit->text().mid(index + txt.length() );
            if ( start.contains( regEnd ) )
                start.replace( regEnd, QString::fromLatin1("") );
            else
                end.replace( regStart,  QString::fromLatin1("") );

            res = start + end;
        }
        _lineEdit->setText( res );
    }
}



void ListSelect::showContextMenu( QListBoxItem* item, const QPoint& pos )
{
    if ( !item ) {
        // click outside any item
        return;
    }

    QPopupMenu menu( this );
    QLabel* title = new QLabel( QString::fromLatin1("<qt><b>%1</b></qt>").arg(item->text()), &menu );
    title->setAlignment( Qt::AlignCenter );
    menu.insertItem( title );
    menu.insertSeparator();

    menu.insertItem( i18n("Delete"), 1 );
    menu.insertItem( i18n("Rename"), 2 );
    int which = menu.exec( pos );
    if ( which == 1 ) {
        int code = KMessageBox::questionYesNo( this, i18n("<qt>Do you really want to delete \"%1\"?<br>"
                                                          "Deleting the item will remove any information about "
                                                          "about it from any image containing the item!</qt>")
                                               .arg(item->text()),
                                               i18n("Really Delete %1?").arg(item->text()) );
        if ( code == KMessageBox::Yes ) {
            Options::instance()->removeOption(optionGroup(), item->text() );
            delete item;
        }
    }
    else if ( which == 2 ) {
        bool ok;
        QString newStr = KInputDialog::getText( i18n("Rename Item"), i18n("Rename %1").arg( item->text() ),
                                                item->text(), &ok, this );

        if ( ok && newStr != item->text() ) {
            int code = KMessageBox::questionYesNo( this, i18n("<qt>Do you really want to rename \"%1\" to \"%2\"?<br>"
                                                              "Doing so will rename \"%3\" "
                                                              "on any image containing it.</qt>")
                                               .arg(item->text()).arg(newStr).arg(item->text()),
                                               i18n("Really Rename %1?").arg(item->text()) );
            if ( code == KMessageBox::Yes ) {
                Options::instance()->renameOption( optionGroup(), item->text(), newStr );
                bool sel = item->isSelected();
                delete item;
                QListBoxText* newItem = new QListBoxText( _listBox, newStr );
                _listBox->setSelected( newItem, sel );
            }
        }
    }
}


void ListSelect::populate()
{
    _label->setText( Options::instance()->textForOptionGroup( _optionGroup ) );
    _listBox->clear();
    QStringList items = Options::instance()->optionValue( _optionGroup );
    _listBox->insertStringList( items );

    // add the groups to the listbox too, but only if the group is not there already, which will be the case
    // if it has ever been selected once.
    QStringList groups = Options::instance()->memberMap().groups( _optionGroup );
    for( QStringList::Iterator it = groups.begin(); it != groups.end(); ++it ) {
        if ( ! items.contains(  *it ) )
            _listBox->insertItem( *it );
    }
}

/**
   When the user presses the right mouse button on the list box to show the
   context menu, then the selection state of the list box will also change,
   which is indeed not his intention. Therefore this event filter will
   block the mouse press events when they come from a right mouse button.
*/
bool ListSelect::eventFilter( QObject* object, QEvent* event )
{
    if ( object == _listBox && event->type() == QEvent::MouseButtonPress &&
         static_cast<QMouseEvent*>(event)->button() == Qt::RightButton )
        return true;
    return QWidget::eventFilter( object, event );
}

#include "listselect.moc"

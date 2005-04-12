/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "listselect.h"
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvalidator.h>
#include "imageinfo.h"
#include <qpopupmenu.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <qapplication.h>
#include "imagedb.h"
#include <kio/job.h>
#include <qtoolbutton.h>
#include <kiconloader.h>
#include <qbuttongroup.h>
#include "categorycollection.h"
#include "membermap.h"
#include <qinputdialog.h>

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


ListSelect::ListSelect( const QString& category, QWidget* parent, const char* name )
    : QWidget( parent,  name ), _category( category )
{
    QVBoxLayout* layout = new QVBoxLayout( this,  6 );

    _label = new QLabel( CategoryCollection::instance()->categoryForName( category )->text(), this );
    _label->setAlignment( AlignCenter );
    layout->addWidget( _label );

    _lineEdit = new CompletableLineEdit( this, QString::fromLatin1( "line edit for %1").arg(category).latin1() );
    layout->addWidget( _lineEdit );

    _listBox = new QListBox( this );
    _listBox->setSelectionMode( QListBox::Multi );
    connect( _listBox, SIGNAL( clicked( QListBoxItem*  ) ),  this,  SLOT( itemSelected( QListBoxItem* ) ) );
    connect( _listBox, SIGNAL( contextMenuRequested( QListBoxItem*, const QPoint& ) ),
             this, SLOT(showContextMenu( QListBoxItem*, const QPoint& ) ) );
    layout->addWidget( _listBox );
    _listBox->installEventFilter( this );

    // Merge CheckBox
    QHBoxLayout* lay2 = new QHBoxLayout( layout, 6 );
    _checkBox = new QCheckBox( QString(),  this );
    lay2->addWidget( _checkBox );
    lay2->addStretch(1);

    // Sorting tool button
    QButtonGroup* grp = new QButtonGroup( this );
    grp->setExclusive( true );
    grp->hide();

    _alphaSort = new QToolButton( this, "_alphaSort" );
    _alphaSort->setIconSet( SmallIcon( QString::fromLatin1( "text" ) ) );
    _alphaSort->setToggleButton( true );
    grp->insert( _alphaSort );

    _dateSort = new QToolButton( this, "_dateSort" );
    _dateSort->setIconSet( SmallIcon( QString::fromLatin1( "date" ) ) );
    _dateSort->setToggleButton( true );
    grp->insert( _dateSort );

    _alphaSort->setOn( Options::ViewSortType() == Options::SortAlpha );
    _dateSort->setOn( Options::ViewSortType() == Options::SortLastUse );
    connect( _dateSort, SIGNAL( clicked() ), this, SLOT( slotSortDate() ) );
    connect( _alphaSort, SIGNAL( clicked() ), this, SLOT( slotSortAlpha() ) );

    lay2->addWidget( _alphaSort );
    lay2->addWidget( _dateSort );

    _lineEdit->setListBox( _listBox );
    connect( _lineEdit, SIGNAL( returnPressed() ),  this,  SLOT( slotReturn() ) );

    populate();

    connect( Options::instance(), SIGNAL( viewSortTypeChanged( Options::ViewSortType ) ),
             this, SLOT( setViewSortType( Options::ViewSortType ) ) );
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
        options->addOption( _category, txt);

        // move item to front
        _listBox->takeItem( item );
        if ( Options::instance()->viewSortType() == Options::SortLastUse ) {
            _listBox->insertItem( item, 0 );
            _listBox->setContentsPos( 0,0 );
        }
        else {
            QListBoxItem* lbi = 0;
            for ( lbi = _listBox->firstItem(); lbi && lbi->text().lower() < txt.lower(); lbi = lbi->next() ) {};

            if ( !lbi ) {
                _listBox->insertItem( item );
            }
            else {
                _listBox->insertItem( item, lbi->prev() );
            }
        }

        _listBox->setSelected( item,  true );
        _lineEdit->clear();
    }
}

QString ListSelect::category() const
{
    return _category;
}

void ListSelect::setSelection( const QStringList& list )
{
    _listBox->clearSelection();

    for( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
        QListBoxItem* item = _listBox->findItem( *it,  ExactMatch );
        if ( !item )  {
            _listBox->insertItem( *it );
            item = _listBox->findItem( *it,  ExactMatch );
            Options::instance()->addOption( _category, *it);
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
    _checkBox->setEnabled( b );
}

bool ListSelect::doMerge() const
{
    return _checkBox->isChecked();
}

bool ListSelect::isAND() const
{
    return _checkBox->isChecked();
}

void ListSelect::setMode( Mode mode )
{
    _mode = mode;
    _lineEdit->setMode( mode );
    if ( mode == SEARCH) {
        QListBoxItem * none = new QListBoxText( 0, ImageDB::NONE() );
        _listBox->insertItem( none, 0 );
	_checkBox->setText( i18n("AND") );
	// OR is a better default choice (the browser can do AND but not OR)
	_checkBox->setChecked( false );
    } else {
	_checkBox->setText( i18n("Merge") );
	_checkBox->setChecked( true );
    }
}


void ListSelect::setViewSortType( Options::ViewSortType tp )
{
    // set sortType and redisplay with new sortType
    QString text = _lineEdit->text();
    QStringList list = selection();
    populate();
    setSelection( list );
    _lineEdit->setText( text );
    setMode( _mode );	// generate the ***NONE*** entry if in search mode

    _alphaSort->setOn( tp == Options::SortAlpha );
    _dateSort->setOn( tp == Options::SortLastUse );
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
    if ( !item ) {
        // click outside any item
        return;
    }

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
		res += isAND() ? QString::fromLatin1("&") : QString::fromLatin1("|") ;
            res += txt;
	    if ( !end.isEmpty() && !end.contains( regStart ) )
		res += isAND() ? QString::fromLatin1("&") : QString::fromLatin1("|") ;
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
    QPopupMenu menu( this, "context popup menu" );

    // click on any item
    QString title = i18n("No Item Selected");
    if ( item )
        title = item->text();

    QLabel* label = new QLabel( QString::fromLatin1("<qt><b>%1</b></qt>").arg(title), &menu );
    label->setAlignment( Qt::AlignCenter );
    menu.insertItem( label );
    menu.insertItem( SmallIcon(QString::fromLatin1("editdelete")), i18n("Delete"), 1 );
    menu.insertItem( i18n("Rename..."), 2 );

    // -------------------------------------------------- Add/Remove member group
    MemberMap memberMap = Options::instance()->memberMap();
    QMap<int, QString> map;
    QPopupMenu* members = new QPopupMenu( &menu );
    members->setCheckable( true );
    menu.insertItem( i18n( "Member Groups" ), members, 5 );
    if ( item ) {
        QStringList grps = memberMap.groups( _category );

        int index = 10;

        for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
            members->insertItem( *it, ++index );
            map.insert( index, *it );
            members->setItemChecked( index, (bool) memberMap.members( _category, *it, true ).contains( item->text() ) );
        }

        if ( !grps.isEmpty() )
            members->insertSeparator();
        members->insertItem( i18n("New Group..." ), 7 );
    }

    // -------------------------------------------------- sort
    QLabel* sortTitle = new QLabel( i18n("<qt><b>Sorting</b></qt>"), &menu );
    sortTitle->setAlignment( Qt::AlignCenter );
    menu.insertItem( sortTitle );
    menu.insertItem( i18n("Usage"), 3 );
    menu.insertItem( i18n("Alphabetical"), 4 );
    menu.setItemChecked(3, Options::instance()->viewSortType() == Options::SortLastUse);
    menu.setItemChecked(4, Options::instance()->viewSortType() == Options::SortAlpha);

    if ( !item ) {
        menu.setItemEnabled( 1, false );
        menu.setItemEnabled( 2, false );
        menu.setItemEnabled( 5, false );
        menu.setItemEnabled( 6, false );
    }

    // -------------------------------------------------- exec
    int which = menu.exec( pos );
    if ( which == 1 ) {
        int code = KMessageBox::warningContinueCancel( this, i18n("<qt>Do you really want to delete \"%1\"?<br>"
                                                          "Deleting the item will remove any information about "
                                                          "about it from any image containing the item.</qt>")
                                               .arg(item->text()),
                                               i18n("Really Delete %1?").arg(item->text()), KGuiItem(i18n("&Delete"),QString::fromLatin1("editdelete")) );
        if ( code == KMessageBox::Continue ) {
            Options::instance()->removeOption(category(), item->text() );
            delete item;
        }
    }
    else if ( which == 2 ) {
        bool ok;
        QString newStr = KInputDialog::getText( i18n("Rename Item"), i18n("Enter new name:"),
                                                item->text(), &ok, this );

        if ( ok && newStr != item->text() ) {
            int code = KMessageBox::questionYesNo( this, i18n("<qt>Do you really want to rename \"%1\" to \"%2\"?<br>"
                                                              "Doing so will rename \"%3\" "
                                                              "on any image containing it.</qt>")
                                               .arg(item->text()).arg(newStr).arg(item->text()),
                                               i18n("Really Rename %1?").arg(item->text()) );
            if ( code == KMessageBox::Yes ) {
                QString oldStr = item->text();
                Options::instance()->renameOption( category(), oldStr, newStr );
                bool sel = item->isSelected();
                delete item;
                QListBoxText* newItem = new QListBoxText( _listBox, newStr );
                _listBox->setSelected( newItem, sel );

                // rename the category image too
                QString oldFile = Options::instance()->fileForCategoryImage( category(), oldStr );
                QString newFile = Options::instance()->fileForCategoryImage( category(), newStr );
                KIO::move( KURL(oldFile), KURL(newFile) );
            }
        }
    }
    else if ( which == 3 ) {
        Options::instance()->setViewSortType( Options::SortLastUse );
    }
    else if ( which == 4 ) {
        Options::instance()->setViewSortType( Options::SortAlpha );
    }
    else if ( which == 7 ) {
        QString group = KInputDialog::getText( i18n("Member Group Name"), i18n("Member group name:") );
        if ( group.isNull() )
            return;
        memberMap.addGroup( _category, group );
        memberMap.addMemberToGroup( _category, group, item->text() );
        Options::instance()->setMemberMap( memberMap );
    }
    else {
        if ( map.contains( which ) ) {
            QString checkedItem = map[which];
            if ( !members->isItemChecked( which ) ) // chosing the item doesn't check it, so this is the value before.
                memberMap.addMemberToGroup( _category, checkedItem, item->text() );
            else
                memberMap.removeMemberFromGroup( _category, checkedItem, item->text() );
            Options::instance()->setMemberMap( memberMap );
        }
    }
}


void ListSelect::populate()
{
    _label->setText( CategoryCollection::instance()->categoryForName( _category )->text() );
    _listBox->clear();
    QStringList items = Options::instance()->optionValueInclGroups( _category );
    _listBox->insertStringList( items );
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

void ListSelect::slotSortDate()
{
    Options::instance()->setViewSortType( Options::SortLastUse );
}

void ListSelect::slotSortAlpha()
{
    Options::instance()->setViewSortType( Options::SortAlpha );
}

#include "listselect.moc"

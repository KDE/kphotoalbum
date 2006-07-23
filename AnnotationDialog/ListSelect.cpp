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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "ListSelect.h"
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvalidator.h>
#include "DB/ImageInfo.h"
#include <qpopupmenu.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <qapplication.h>
#include "DB/ImageDB.h"
#include <kio/job.h>
#include <qtoolbutton.h>
#include <kiconloader.h>
#include <qbuttongroup.h>
#include "DB/CategoryCollection.h"
#include "DB/MemberMap.h"
#include <qinputdialog.h>
#include <qlistview.h>
#include <qheader.h>
#include <Utilities/Set.h>
#include "CompletableLineEdit.h"
#include "DB/CategoryItem.h"
#include "ListViewItemHider.h"
#include "KeyClickListener.h"
#include "ShowSelectionOnlyManager.h"

using namespace AnnotationDialog;

AnnotationDialog::ListSelect::ListSelect( const QString& category, QWidget* parent, const char* name )
    : QWidget( parent,  name ), _category( category )
{
    QVBoxLayout* layout = new QVBoxLayout( this,  6 );

    _label = new QLabel( DB::ImageDB::instance()->categoryCollection()->categoryForName( category )->text(), this );
    _label->setAlignment( AlignCenter );
    layout->addWidget( _label );

    _lineEdit = new CompletableLineEdit( this, QString::fromLatin1( "line edit for %1").arg(category).latin1() );
    layout->addWidget( _lineEdit );

    _listView = new QListView( this );
    _listView->addColumn( QString::fromLatin1( "items" ) );
    _listView->header()->setStretchEnabled( true );
    _listView->header()->hide();
    _listView->setSelectionMode( QListView::Multi );
    connect( _listView, SIGNAL( clicked( QListViewItem*  ) ),  this,  SLOT( itemSelected( QListViewItem* ) ) );
    connect( _listView, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
             this, SLOT(showContextMenu( QListViewItem*, const QPoint& ) ) );
    layout->addWidget( _listView );
    _listView->viewport()->installEventFilter( this );

    // Merge CheckBox
    QHBoxLayout* lay2 = new QHBoxLayout( layout, 6 );
    _checkBox = new QCheckBox( QString(),  this );
    connect( _checkBox, SIGNAL( stateChanged( int ) ), this,
            SLOT(checkBoxStateChanged(int) ) );
    lay2->addWidget( _checkBox );
    lay2->addStretch(1);

    // Merge CheckBox
    _removeCheckBox = new QCheckBox( QString(),  this );
    connect( _removeCheckBox, SIGNAL( stateChanged( int ) ), this,
            SLOT(removeCheckBoxStateChanged(int) ) );
    lay2->addWidget( _removeCheckBox );
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

    _alphaSort->setOn( Settings::ViewSortType() == Settings::SortAlpha );
    _dateSort->setOn( Settings::ViewSortType() == Settings::SortLastUse );
    connect( _dateSort, SIGNAL( clicked() ), this, SLOT( slotSortDate() ) );
    connect( _alphaSort, SIGNAL( clicked() ), this, SLOT( slotSortAlpha() ) );

    lay2->addWidget( _alphaSort );
    lay2->addWidget( _dateSort );

    _lineEdit->setListView( _listView );

    connect( _lineEdit, SIGNAL( returnPressed() ),  this,  SLOT( slotReturn() ) );

    populate();

    connect( Settings::SettingsData::instance(), SIGNAL( viewSortTypeChanged( Settings::ViewSortType ) ),
             this, SLOT( setViewSortType( Settings::ViewSortType ) ) );

    KeyClickListener* listener = new KeyClickListener( Key_Control, this );
    connect( &ShowSelectionOnlyManager::instance(), SIGNAL( limitToSelected() ), this, SLOT(limitToSelection() ) );
    connect( &ShowSelectionOnlyManager::instance(), SIGNAL( broaden() ), this, SLOT( showAllChildren() ) );
    connect( listener, SIGNAL( keyClicked() ), &ShowSelectionOnlyManager::instance(), SLOT( toggle() ) );

    listener = new KeyClickListener( Key_Alt, this );
    connect( listener, SIGNAL( keyClicked() ), this, SLOT( toggleSortType() ) );

    listener = new KeyClickListener( Key_Meta, this );
    connect( listener, SIGNAL( keyClicked() ), this, SLOT( toggleSortType() ) );

}

void AnnotationDialog::ListSelect::checkBoxStateChanged( int )
{
    if (_checkBox->isChecked() && _removeCheckBox->isChecked())
        _removeCheckBox->setChecked(false);
}

void AnnotationDialog::ListSelect::removeCheckBoxStateChanged( int )
{
    QString txt =
        i18n("<qt><p>By checking this checkbox, any anotation you make will actually be removed from the images, "
             "rather than added to them.</p>"
             "<p>This is really just a tool for removing a tag that you by accicdent added to a number of images.</p>"
             "<p>are you sure you want that?</p></qt>" );
    if ( _removeCheckBox->isChecked() ) {
        int ret = KMessageBox::warningContinueCancel( this, txt, i18n("Mass removal of tags"),KStdGuiItem::cont(),
                                                      QString::fromLatin1("massremoval") );
        if ( ret == KMessageBox::Cancel )
            _removeCheckBox->setChecked( false );
    }

    if (_checkBox->isChecked() && _removeCheckBox->isChecked())
        _checkBox->setChecked(false);
}

void AnnotationDialog::ListSelect::slotReturn()
{
    if ( _mode == INPUT )  {
        QString txt = _lineEdit->text();
        if ( txt.isEmpty() )
            return;

        DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->addItem( txt);

        QStringList sel = selection();
        sel.append( txt );
        populate();
        setSelection(sel);

        _lineEdit->clear();
    }
}

QString AnnotationDialog::ListSelect::category() const
{
    return _category;
}

void AnnotationDialog::ListSelect::setSelection( const QStringList& list )
{
    // PENDING(blackie) change method to take a set
    Set<QString> selection( list );
    for ( QListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt ) {
        (*itemIt)->setSelected( selection.contains( (*itemIt)->text(0) ) );
    }

    _lineEdit->clear();
}

QStringList AnnotationDialog::ListSelect::selection()
{
    // PENDING(blackie) should this method return a set?
    QStringList list;
    for ( QListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt ) {
        if ( (*itemIt)->isSelected() )
            list.append( (*itemIt)->text(0) );
    }
    return list;
}

void AnnotationDialog::ListSelect::setShowMergeCheckbox( bool b )
{
    // PENDING(blackie) 19 Mar. 2006 20:29 -- Jesper K. Pedersen
    // This is really a crual hack and should be removed after next release.
    // We should instead extend the Mode enum to say InputSingleConfig/InputMultiConfig/Search
    _checkBox->setEnabled( b );
    _removeCheckBox->setEnabled( b );
}

bool AnnotationDialog::ListSelect::doMerge() const
{
    return _checkBox->isChecked();
}

bool AnnotationDialog::ListSelect::doRemove() const
{
    return _removeCheckBox->isChecked();
}

bool AnnotationDialog::ListSelect::isAND() const
{
    return _checkBox->isChecked();
}

void AnnotationDialog::ListSelect::setMode( Mode mode )
{
    _mode = mode;
    _lineEdit->setMode( mode );
    if ( mode == SEARCH) {
        // "0" below is sorting key which ensures that None is always at top.
        new QListViewItem( _listView, DB::ImageDB::NONE(), QString::fromLatin1("0") );
	_checkBox->setText( i18n("AND") );
	// OR is a better default choice (the browser can do AND but not OR)
	_checkBox->setChecked( false );
        _removeCheckBox->hide();
    } else {
	_checkBox->setText( i18n("Merge") );
	_checkBox->setChecked( true );
        _removeCheckBox->setText( i18n("Mass Remove") );
        _removeCheckBox->setChecked( false );
        _removeCheckBox->show();
    }
}


void AnnotationDialog::ListSelect::setViewSortType( Settings::ViewSortType tp )
{
    // set sortType and redisplay with new sortType
    QString text = _lineEdit->text();
    QStringList list = selection();
    populate();
    setSelection( list );
    _lineEdit->setText( text );
    setMode( _mode );	// generate the ***NONE*** entry if in search mode

    _alphaSort->setOn( tp == Settings::SortAlpha );
    _dateSort->setOn( tp == Settings::SortLastUse );
}


QString AnnotationDialog::ListSelect::text() const
{
    return _lineEdit->text();
}

void AnnotationDialog::ListSelect::setText( const QString& text )
{
    _lineEdit->setText( text );
    _listView->clearSelection();
}

void AnnotationDialog::ListSelect::itemSelected( QListViewItem* item )
{
    if ( !item ) {
        // click outside any item
        return;
    }

    if ( _mode == SEARCH )  {
        QString txt = item->text(0);
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
    else {
        _lineEdit->clear();
        showAllChildren();
    }
}


void AnnotationDialog::ListSelect::showContextMenu( QListViewItem* item, const QPoint& pos )
{
    QPopupMenu menu( this, "context popup menu" );

    // click on any item
    QString title = i18n("No Item Selected");
    if ( item )
        title = item->text(0);

    QLabel* label = new QLabel( QString::fromLatin1("<qt><b>%1</b></qt>").arg(title), &menu );
    label->setAlignment( Qt::AlignCenter );
    menu.insertItem( label );
    menu.insertItem( SmallIcon(QString::fromLatin1("editdelete")), i18n("Delete"), 1 );
    menu.insertItem( i18n("Rename..."), 2 );

    // -------------------------------------------------- Add/Remove member group
    DB::MemberMap memberMap = DB::ImageDB::instance()->memberMap();
    QMap<int, QString> map;
    QPopupMenu* members = new QPopupMenu( &menu );
    members->setCheckable( true );
    menu.insertItem( i18n( "Super Categories" ), members, 5 );
    if ( item ) {
        QStringList grps = memberMap.groups( _category );

        int index = 10;

        for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
            members->insertItem( *it, ++index );
            map.insert( index, *it );
            members->setItemChecked( index, (bool) memberMap.members( _category, *it, true ).contains( item->text(0) ) );
        }

        if ( !grps.isEmpty() )
            members->insertSeparator();
        members->insertItem( i18n("New Category..." ), 7 );
    }
    menu.insertItem( i18n( "Create Subcategory..." ), 8 );

    // -------------------------------------------------- sort
    QLabel* sortTitle = new QLabel( i18n("<qt><b>Sorting</b></qt>"), &menu );
    sortTitle->setAlignment( Qt::AlignCenter );
    menu.insertItem( sortTitle );
    menu.insertItem( i18n("Usage"), 3 );
    menu.insertItem( i18n("Alphabetical"), 4 );
    menu.setItemChecked(3, Settings::SettingsData::instance()->viewSortType() == Settings::SortLastUse);
    menu.setItemChecked(4, Settings::SettingsData::instance()->viewSortType() == Settings::SortAlpha);

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
                                               .arg(item->text(0)),
                                               i18n("Really Delete %1?").arg(item->text(0)), KGuiItem(i18n("&Delete"),QString::fromLatin1("editdelete")) );
        if ( code == KMessageBox::Continue ) {
            DB::ImageDB::instance()->categoryCollection()->categoryForName(category())->removeItem( item->text(0) );
            delete item;
        }
    }
    else if ( which == 2 ) {
        bool ok;
        QString newStr = KInputDialog::getText( i18n("Rename Item"), i18n("Enter new name:"),
                                                item->text(0), &ok, this );

        if ( ok && newStr != item->text(0) ) {
            int code = KMessageBox::questionYesNo( this, i18n("<qt>Do you really want to rename \"%1\" to \"%2\"?<br>"
                                                              "Doing so will rename \"%3\" "
                                                              "on any image containing it.</qt>")
                                               .arg(item->text(0)).arg(newStr).arg(item->text(0)),
                                               i18n("Really Rename %1?").arg(item->text(0)) );
            if ( code == KMessageBox::Yes ) {
                QString oldStr = item->text(0);
                DB::ImageDB::instance()->categoryCollection()->categoryForName( category() )->renameItem( oldStr, newStr );
                bool sel = item->isSelected();
                delete item;
                QListViewItem* newItem = new QListViewItem( _listView, newStr );
                newItem->setSelected( sel );

                // rename the category image too
                QString oldFile = Settings::SettingsData::instance()->fileForCategoryImage( category(), oldStr );
                QString newFile = Settings::SettingsData::instance()->fileForCategoryImage( category(), newStr );
                KIO::move( KURL(oldFile), KURL(newFile) );
            }
        }
    }
    else if ( which == 3 ) {
        Settings::SettingsData::instance()->setViewSortType( Settings::SortLastUse );
    }
    else if ( which == 4 ) {
        Settings::SettingsData::instance()->setViewSortType( Settings::SortAlpha );
    }
    else if ( which == 7 ) {
        QString superCategory = KInputDialog::getText( i18n("New Super Category"), i18n("New Super Category Name:") );
        if ( superCategory.isNull() )
            return;
        memberMap.addGroup( _category, superCategory );
        memberMap.addMemberToGroup( _category, superCategory, item->text(0) );
        DB::ImageDB::instance()->setMemberMap( memberMap );
        rePopulate();
    }
    else if ( which == 8 ) {
        QString subCategory = KInputDialog::getText( i18n("New Sub Category"), i18n("New Sub Category Name:") );
        if ( subCategory.isNull() )
            return;

        DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->addItem( subCategory );
        memberMap.addGroup( _category, item->text(0) );
        memberMap.addMemberToGroup( _category, item->text(0), subCategory );
        DB::ImageDB::instance()->setMemberMap( memberMap );
        if ( _mode == INPUT ) {
            qDebug("OK adding %s", subCategory.latin1());
            DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->addItem( subCategory );
        }

        // PENDING(blackie) select the newly added item
        rePopulate();
    }
    else {
        if ( map.contains( which ) ) {
            QString checkedItem = map[which];
            if ( !members->isItemChecked( which ) ) // chosing the item doesn't check it, so this is the value before.
                memberMap.addMemberToGroup( _category, checkedItem, item->text(0) );
            else
                memberMap.removeMemberFromGroup( _category, checkedItem, item->text(0) );
            DB::ImageDB::instance()->setMemberMap( memberMap );
        }
    }
}


void AnnotationDialog::ListSelect::insertItems( DB::CategoryItem* item, QListViewItem* parent )
{
    for( QValueList<DB::CategoryItem*>::ConstIterator subcategoryIt = item->_subcategories.begin(); subcategoryIt != item->_subcategories.end(); ++subcategoryIt ) {
        QListViewItem* newItem = 0;

        if ( parent == 0 )
            newItem = new QListViewItem( _listView, (*subcategoryIt)->_name );
        else
            newItem = new QListViewItem( parent, (*subcategoryIt)->_name );

        newItem->setOpen( true );
        insertItems( (*subcategoryIt), newItem );
    }
}

void AnnotationDialog::ListSelect::populate()
{
    DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( _category );
    _label->setText( category->text() );
    _listView->clear();

    if ( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlpha )
        populateAlphabetically();
    else
        populateMRU();
}

/**
   When the user presses the right mouse button on the list view to show the
   context menu, then the selection state of the list view item under point will also change,
   which is indeed not his intention. Therefore this event filter will
   block the mouse press events when they come from a right mouse button.
*/
bool AnnotationDialog::ListSelect::eventFilter( QObject* object, QEvent* event )
{
    if ( object == _listView->viewport() && event->type() == QEvent::MouseButtonPress &&
         static_cast<QMouseEvent*>(event)->button() == Qt::RightButton )
        return true;
    return QWidget::eventFilter( object, event );
}

void AnnotationDialog::ListSelect::slotSortDate()
{
    Settings::SettingsData::instance()->setViewSortType( Settings::SortLastUse );
}

void AnnotationDialog::ListSelect::slotSortAlpha()
{
    Settings::SettingsData::instance()->setViewSortType( Settings::SortAlpha );
}

void AnnotationDialog::ListSelect::rePopulate()
{
    const QStringList sel = selection();
    populate();
    setSelection( sel );
}

void AnnotationDialog::ListSelect::showOnlyItemsMatching( const QString& text )
{
    ListViewTextMatchHider dummy( text, _listView );
    ShowSelectionOnlyManager::instance().unlimitFromSelection();
}

void AnnotationDialog::ListSelect::populateAlphabetically()
{
    DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName( _category );
    KSharedPtr<DB::CategoryItem> item = category->itemsCategories();

    insertItems( item, 0 );
    _listView->setSorting( 0 );
}

void AnnotationDialog::ListSelect::populateMRU()
{
    QStringList items = DB::ImageDB::instance()->categoryCollection()->categoryForName( _category )->itemsInclCategories();

    int index = 100000; // This counter will be converted to a string, and compared, and we don't want "1111" to be less than "2"
    for( QStringList::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        ++index;
        new QListViewItem( _listView, *itemIt, QString::number( index ) );
    }

    _listView->setSorting( 1 );
}

void AnnotationDialog::ListSelect::toggleSortType()
{
    Settings::SettingsData* data = Settings::SettingsData::instance();
    if ( data->viewSortType() == Settings::SortLastUse )
        data->setViewSortType( Settings::SortAlpha );
    else
        data->setViewSortType( Settings::SortLastUse );
}

void AnnotationDialog::ListSelect::limitToSelection()
{
    if ( _mode != ListSelect::INPUT )
        return;

    ListViewSelectionHider dummy( _listView );
}

void AnnotationDialog::ListSelect::showAllChildren()
{
    showOnlyItemsMatching( QString::null );
}

#include "ListSelect.moc"

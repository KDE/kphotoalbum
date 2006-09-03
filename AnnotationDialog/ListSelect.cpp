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
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvalidator.h>
#include <qpopupmenu.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include "DB/ImageDB.h"
#include <kio/job.h>
#include <qtoolbutton.h>
#include <kiconloader.h>
#include <qbuttongroup.h>
#include "DB/CategoryCollection.h"
#include "DB/MemberMap.h"
#include <qlistview.h>
#include <qheader.h>
#include <Utilities/Set.h>
#include "CompletableLineEdit.h"
#include "DB/CategoryItem.h"
#include "ListViewItemHider.h"
#include "ShowSelectionOnlyManager.h"
#include "CategoryListView/DragableListView.h"
#include "CategoryListView/CheckDropItem.h"
#include <qradiobutton.h>

using namespace AnnotationDialog;
using CategoryListView::CheckDropItem;

AnnotationDialog::ListSelect::ListSelect( const DB::CategoryPtr& category, QWidget* parent, const char* name )
    : QWidget( parent,  name ), _category( category )
{
    QVBoxLayout* layout = new QVBoxLayout( this,  6 );

    _label = new QLabel( _category->text(), this );
    _label->setAlignment( AlignCenter );
    layout->addWidget( _label );

    _lineEdit = new CompletableLineEdit( this, QString::fromLatin1( "line edit for %1").arg(_category->name()).latin1() );
    _label->setBuddy( _lineEdit );
    layout->addWidget( _lineEdit );

    _listView = new CategoryListView::DragableListView( _category, this );
    _listView->viewport()->setAcceptDrops( true );
    _listView->addColumn( QString::fromLatin1( "items" ) );
    _listView->header()->setStretchEnabled( true );
    _listView->header()->hide();
    _listView->setSelectionMode( QListView::Extended );
    connect( _listView, SIGNAL( clicked( QListViewItem*  ) ),  this,  SLOT( itemSelected( QListViewItem* ) ) );
    connect( _listView, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
             this, SLOT(showContextMenu( QListViewItem*, const QPoint& ) ) );
    connect( _listView, SIGNAL( itemsChanged() ), this, SLOT( rePopulate() ) );

    layout->addWidget( _listView );
    _listView->viewport()->installEventFilter( this );

    // Merge CheckBox
    QHBoxLayout* lay2 = new QHBoxLayout( layout, 6 );
    QButtonGroup* group = new QButtonGroup( this );
    group->hide();
    _or = new QRadioButton( i18n("or"), this );
    _and = new QRadioButton( i18n("and"), this );
    group->insert( _and );
    group->insert( _or );
    lay2->addWidget( _or );
    lay2->addWidget( _and );
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

    connect( &ShowSelectionOnlyManager::instance(), SIGNAL( limitToSelected() ), this, SLOT(limitToSelection() ) );
    connect( &ShowSelectionOnlyManager::instance(), SIGNAL( broaden() ), this, SLOT( showAllChildren() ) );
}

void AnnotationDialog::ListSelect::slotReturn()
{
    if ( isInputMode() )  {
        QString txt = _lineEdit->text();
        if ( txt.isEmpty() )
            return;

        _category->addItem( txt);

        QListViewItem* item = _listView->findItem( txt, 0 );
        if ( item )
            static_cast<QCheckListItem*>(item)->setOn( true );
        else
            Q_ASSERT( false );


        _lineEdit->clear();
    }
}

QString AnnotationDialog::ListSelect::category() const
{
    return _category->name();
}

void AnnotationDialog::ListSelect::setSelection( const StringSet& on, const StringSet& partiallyOn )
{
    for ( QListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt ) {
        QCheckListItem* item = static_cast<QCheckListItem*>(*itemIt);
        if ( partiallyOn.contains( item->text(0) ) )
            item->setState( QCheckListItem::NoChange );
        else
            item->setOn( on.contains( item->text(0) ) );

        // static_cast<QCheckListItem*>(*itemIt)->setOn( selection.contains( (*itemIt)->text(0) ) );
        // static_cast<QCheckListItem*>(*itemIt)->setState( QCheckListItem::NoChange );
        _listView->repaintItem(*itemIt);
    }

    _lineEdit->clear();
}

bool AnnotationDialog::ListSelect::isAND() const
{
    return _and->isChecked();
}

void AnnotationDialog::ListSelect::setMode( UsageMode mode )
{
    _mode = mode;
    _lineEdit->setMode( mode );
    if ( mode == SearchMode ) {
        // "0" below is sorting key which ensures that None is always at top.
        CheckDropItem* item = new CheckDropItem( _listView, DB::ImageDB::NONE(), QString::fromLatin1("0") );
        configureItem( item );
        _and->show();
        _or->show();
        _or->setChecked( true );
    } else {
	_and->hide();
        _or->hide();
    }
    for ( QListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt )
        configureItem( dynamic_cast<CategoryListView::CheckDropItem*>(*itemIt) );
}


void AnnotationDialog::ListSelect::setViewSortType( Settings::ViewSortType tp )
{
    // set sortType and redisplay with new sortType
    QString text = _lineEdit->text();
    rePopulate();
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

    if ( _mode == SearchMode )  {
        QString txt = item->text(0);
        QString res;
        QRegExp regEnd( QString::fromLatin1("\\s*[&|!]\\s*$") );
        QRegExp regStart( QString::fromLatin1("^\\s*[&|!]\\s*") );
        if ( static_cast<QCheckListItem*>(item)->isOn() )  {
            int matchPos = _lineEdit->text().find( txt );
            if ( matchPos != -1 )
                return;

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
    DB::MemberMap& memberMap = DB::ImageDB::instance()->memberMap();
    QMap<int, QString> map;
    QPopupMenu* members = new QPopupMenu( &menu );
    members->setCheckable( true );
    menu.insertItem( i18n( "Super Categories" ), members, 5 );
    if ( item ) {
        QStringList grps = memberMap.groups( _category->name() );

        int index = 10;

        for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
            members->insertItem( *it, ++index );
            map.insert( index, *it );
            members->setItemChecked( index, (bool) memberMap.members( _category->name(), *it, true ).contains( item->text(0) ) );
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
            _category->removeItem( item->text(0) );
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
                _category->renameItem( oldStr, newStr );
                bool sel = static_cast<QCheckListItem*>(item)->isOn();
                delete item;
                CheckDropItem* newItem = new CheckDropItem( _listView, newStr, QString::null );
                newItem->setOn( sel );
                configureItem( newItem );

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
        memberMap.addGroup( _category->name(), superCategory );
        memberMap.addMemberToGroup( _category->name(), superCategory, item->text(0) );
        //DB::ImageDB::instance()->setMemberMap( memberMap );
        rePopulate();
    }
    else if ( which == 8 ) {
        QString subCategory = KInputDialog::getText( i18n("New Sub Category"), i18n("New Sub Category Name:") );
        if ( subCategory.isNull() )
            return;

         _category->addItem( subCategory );
         memberMap.addGroup( _category->name(), item->text(0) );
         memberMap.addMemberToGroup( _category->name(), item->text(0), subCategory );
         //DB::ImageDB::instance()->setMemberMap( memberMap );
        if ( isInputMode() )
            _category->addItem( subCategory );

        rePopulate();
        if ( isInputMode() ) {
            QListViewItem* item = _listView->findItem( subCategory, 0 );
            if ( item )
                static_cast<QCheckListItem*>(item)->setOn( true );
            else
                Q_ASSERT( false );
        }
    }
    else {
        if ( map.contains( which ) ) {
            QString checkedItem = map[which];
            if ( !members->isItemChecked( which ) ) // chosing the item doesn't check it, so this is the value before.
                memberMap.addMemberToGroup( _category->name(), checkedItem, item->text(0) );
            else
                memberMap.removeMemberFromGroup( _category->name(), checkedItem, item->text(0) );
            //DB::ImageDB::instance()->setMemberMap( memberMap );
            rePopulate();
        }
    }
}


void AnnotationDialog::ListSelect::insertItems( DB::CategoryItem* item, QListViewItem* parent )
{
    for( QValueList<DB::CategoryItem*>::ConstIterator subcategoryIt = item->_subcategories.begin(); subcategoryIt != item->_subcategories.end(); ++subcategoryIt ) {
        CheckDropItem* newItem = 0;

        if ( parent == 0 )
            newItem = new CheckDropItem( _listView, (*subcategoryIt)->_name, QString::null );
        else
            newItem = new CheckDropItem( _listView, parent, (*subcategoryIt)->_name, QString::null );

        newItem->setOpen( true );
        configureItem( newItem );

        insertItems( (*subcategoryIt), newItem );
    }
}

void AnnotationDialog::ListSelect::populate()
{
    _label->setText( _category->text() );
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
    const int x = _listView->contentsX();
    const int y = _listView->contentsY();

    const StringSet on = itemsOn();
    const StringSet noChange = itemsUnchanged();
    populate();
    setSelection( on, noChange );

    _listView->setContentsPos( x, y );
}

void AnnotationDialog::ListSelect::showOnlyItemsMatching( const QString& text )
{
    ListViewTextMatchHider dummy( text, true, _listView );
    ShowSelectionOnlyManager::instance().unlimitFromSelection();
}

void AnnotationDialog::ListSelect::populateAlphabetically()
{
    KSharedPtr<DB::CategoryItem> item = _category->itemsCategories();

    insertItems( item, 0 );
    _listView->setSorting( 0 );
}

void AnnotationDialog::ListSelect::populateMRU()
{
    QStringList items = _category->itemsInclCategories();

    int index = 100000; // This counter will be converted to a string, and compared, and we don't want "1111" to be less than "2"
    for( QStringList::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        ++index;
        CheckDropItem* item = new CheckDropItem( _listView, *itemIt, QString::number( index ) );
        configureItem( item );
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
    if ( !isInputMode() )
        return;

    ListViewCheckedHider dummy( _listView );
}

void AnnotationDialog::ListSelect::showAllChildren()
{
    showOnlyItemsMatching( QString::null );
}


void AnnotationDialog::ListSelect::configureItem( CategoryListView::CheckDropItem* item )
{
    bool isDNDAllowed = Settings::SettingsData::instance()->viewSortType() == Settings::SortAlpha;
    item->setDNDEnabled( isDNDAllowed );
    item->setTristate( _mode == InputMultiImageConfigMode );
}

bool AnnotationDialog::ListSelect::isInputMode() const
{
    return _mode != SearchMode;
}

StringSet AnnotationDialog::ListSelect::itemsOn() const
{
    return itemsOfState( QCheckListItem::On );
}

StringSet AnnotationDialog::ListSelect::itemsOff() const
{
    return itemsOfState( QCheckListItem::Off );
}

StringSet AnnotationDialog::ListSelect::itemsOfState( QCheckListItem::ToggleState state ) const
{
    StringSet res;
    for ( QListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt ) {
        if ( static_cast<QCheckListItem*>(*itemIt)->state() == state )
            res.insert( (*itemIt)->text(0) );
    }
    return res;
}

StringSet AnnotationDialog::ListSelect::itemsUnchanged() const
{
    return itemsOfState( QCheckListItem::NoChange );
}

#include "ListSelect.moc"

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

#include "ListSelect.h"
#include <qlayout.h>
#include <QLabel>
#include <QMenu>
#include <QList>
#include <QMouseEvent>
#include <klocale.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include "DB/ImageDB.h"
#include <kio/copyjob.h>
#include <qtoolbutton.h>
#include <QButtonGroup>
#include "DB/MemberMap.h"
#include <q3listview.h>
#include <q3header.h>
#include <Utilities/Set.h>
#include "CompletableLineEdit.h"
#include "DB/CategoryItem.h"
#include "ListViewItemHider.h"
#include "ShowSelectionOnlyManager.h"
#include "CategoryListView/DragableListView.h"
#include "CategoryListView/CheckDropItem.h"
#include <qradiobutton.h>
#include <QWidgetAction>

using namespace AnnotationDialog;
using CategoryListView::CheckDropItem;

AnnotationDialog::ListSelect::ListSelect( const DB::CategoryPtr& category, QWidget* parent )
    : QWidget( parent ), _category( category ), _baseTitle( )
{
    QVBoxLayout* layout = new QVBoxLayout( this );

    _lineEdit = new CompletableLineEdit( this );
    _lineEdit->setProperty( "FocusCandidate", true );
    _lineEdit->setProperty( "WantsFocus", true );
    _lineEdit->setObjectName( category->name() );
    layout->addWidget( _lineEdit );

    _listView = new CategoryListView::DragableListView( _category, this );
    _listView->viewport()->setAcceptDrops( true );
    _listView->addColumn( QString::fromLatin1( "items" ) );
    _listView->header()->setStretchEnabled( true );
    _listView->header()->hide();
    _listView->setSelectionMode( Q3ListView::Extended );
    connect( _listView, SIGNAL( clicked( Q3ListViewItem*  ) ),  this,  SLOT( itemSelected( Q3ListViewItem* ) ) );
    connect( _listView, SIGNAL( contextMenuRequested( Q3ListViewItem*, const QPoint&, int ) ),
             this, SLOT(showContextMenu( Q3ListViewItem*, const QPoint& ) ) );
    connect( _listView, SIGNAL( itemsChanged() ), this, SLOT( rePopulate() ) );
    connect( _listView, SIGNAL( selectionChanged() ), this, SLOT( updateSelectionCount() ) );

    layout->addWidget( _listView );
    _listView->viewport()->installEventFilter( this );

    // Merge CheckBox
    QHBoxLayout* lay2 = new QHBoxLayout;
    layout->addLayout( lay2 );

    _or = new QRadioButton( i18n("or"), this );
    _and = new QRadioButton( i18n("and"), this );
    lay2->addWidget( _or );
    lay2->addWidget( _and );
    lay2->addStretch(1);

    // Sorting tool button
    QButtonGroup* grp = new QButtonGroup( this );
    grp->setExclusive( true );

    _alphaTreeSort = new QToolButton;
    _alphaTreeSort->setIcon( SmallIcon( QString::fromLatin1( "view-list-tree" ) ) );
    _alphaTreeSort->setCheckable( true );
    _alphaTreeSort->setToolTip( i18n("Sort Alphabetically (Tree)") );
    grp->addButton( _alphaTreeSort );

    _alphaFlatSort = new QToolButton;
    _alphaFlatSort->setIcon( SmallIcon( QString::fromLatin1( "draw-text" ) ) );
    _alphaFlatSort->setCheckable( true );
    _alphaFlatSort->setToolTip( i18n("Sort Alphabetically (Flat)") );
    grp->addButton( _alphaFlatSort );

    _dateSort = new QToolButton;
    _dateSort->setIcon( SmallIcon( QString::fromLatin1( "x-office-calendar" ) ) );
    _dateSort->setCheckable( true );
    _dateSort->setToolTip( i18n("Sort by date") );
    grp->addButton( _dateSort );

    _showSelectedOnly = new QToolButton;
    _showSelectedOnly->setIcon( SmallIcon( QString::fromLatin1( "view-filter" ) ) );
    _showSelectedOnly->setCheckable( true );
    _showSelectedOnly->setToolTip( i18n("Show only selected Ctrl+S") );
	_showSelectedOnly->setChecked( ShowSelectionOnlyManager::instance().selectionIsLimited() );

    _alphaTreeSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree );
    _alphaFlatSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaFlat );
    _dateSort->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortLastUse );
    connect( _dateSort, SIGNAL( clicked() ), this, SLOT( slotSortDate() ) );
    connect( _alphaTreeSort, SIGNAL( clicked() ), this, SLOT( slotSortAlphaTree() ) );
    connect( _alphaFlatSort, SIGNAL( clicked() ), this, SLOT( slotSortAlphaFlat() ) );
    connect( _showSelectedOnly, SIGNAL( clicked() ), &ShowSelectionOnlyManager::instance(), SLOT( toggle() ) );

    lay2->addWidget( _alphaTreeSort );
    lay2->addWidget( _alphaFlatSort );
    lay2->addWidget( _dateSort );
    lay2->addWidget( _showSelectedOnly );

    _lineEdit->setListView( _listView );

    connect( _lineEdit, SIGNAL( returnPressed() ),  this,  SLOT( slotReturn() ) );

    populate();

    connect( Settings::SettingsData::instance(), SIGNAL( viewSortTypeChanged( Settings::ViewSortType ) ),
             this, SLOT( setViewSortType( Settings::ViewSortType ) ) );
    connect( Settings::SettingsData::instance(), SIGNAL( matchTypeChanged( AnnotationDialog::MatchType ) ),
             this, SLOT( updateListview( ) ) );

    connect( &ShowSelectionOnlyManager::instance(), SIGNAL( limitToSelected() ), this, SLOT(limitToSelection() ) );
    connect( &ShowSelectionOnlyManager::instance(), SIGNAL( broaden() ), this, SLOT( showAllChildren() ) );
}

void AnnotationDialog::ListSelect::slotReturn()
{
    if ( isInputMode() )  {
        QString txt = _lineEdit->text().trimmed();
        if ( txt.isEmpty() )
            return;

        _category->addItem( txt);
        rePopulate();

        Q3ListViewItem* item = _listView->findItem( txt, 0 );
        if ( item )
            static_cast<Q3CheckListItem*>(item)->setOn( true );
        else
            Q_ASSERT( false );


        _lineEdit->clear();
    }
    updateSelectionCount();
}

QString AnnotationDialog::ListSelect::category() const
{
    return _category->name();
}

void AnnotationDialog::ListSelect::setSelection( const StringSet& on, const StringSet& partiallyOn )
{
    for ( Q3ListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt ) {
        Q3CheckListItem* item = static_cast<Q3CheckListItem*>(*itemIt);
        if ( partiallyOn.contains( item->text(0) ) )
            item->setState( Q3CheckListItem::NoChange );
        else
            item->setOn( on.contains( item->text(0) ) );
        _listView->repaintItem(*itemIt);
    }

    _lineEdit->clear();
    updateSelectionCount();
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
        _showSelectedOnly->hide();
    } else {
        _and->hide();
        _or->hide();
        _showSelectedOnly->show();
    }
    for ( Q3ListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt )
        configureItem( dynamic_cast<CategoryListView::CheckDropItem*>(*itemIt) );

    // ensure that the selection count indicator matches the current mode:
    updateSelectionCount();
}


void AnnotationDialog::ListSelect::setViewSortType( Settings::ViewSortType tp )
{
    showAllChildren();

    // set sortType and redisplay with new sortType
    QString text = _lineEdit->text();
    rePopulate();
    _lineEdit->setText( text );
    setMode( _mode );	// generate the ***NONE*** entry if in search mode

    _alphaTreeSort->setChecked( tp == Settings::SortAlphaTree );
    _alphaFlatSort->setChecked( tp == Settings::SortAlphaFlat );
    _dateSort->setChecked( tp == Settings::SortLastUse );
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

void AnnotationDialog::ListSelect::itemSelected( Q3ListViewItem* item )
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
        if ( static_cast<Q3CheckListItem*>(item)->isOn() )  {
            int matchPos = _lineEdit->text().indexOf( txt );
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
            int index = _lineEdit->text().indexOf( txt );
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
        ensureAllInstancesAreStateChanged( item );
    }
}


void AnnotationDialog::ListSelect::showContextMenu( Q3ListViewItem* item, const QPoint& pos )
{
    QMenu* menu = new QMenu( this );

    // click on any item
    QString title = i18n("No Item Selected");
    if ( item )
        title = item->text(0);

    QLabel* label = new QLabel( QString::fromLatin1("<b>%1</b>").arg(title), menu );
    label->setAlignment( Qt::AlignCenter );
    QWidgetAction* action = new QWidgetAction(menu);
    action->setDefaultWidget( label );
    menu->addAction(action);

    QAction* deleteAction = menu->addAction( SmallIcon(QString::fromLatin1("edit-delete")), i18n("Delete") );
    QAction* renameAction = menu->addAction( i18n("Rename...") );

    QLabel* categoryTitle = new QLabel( i18n("<b>Sub Categories</b>"), menu );
    categoryTitle->setAlignment( Qt::AlignCenter );
    action = new QWidgetAction( menu );
    action->setDefaultWidget( categoryTitle );
    menu->addAction( action );

    // -------------------------------------------------- Add/Remove member group
    DB::MemberMap& memberMap = DB::ImageDB::instance()->memberMap();
    QMenu* members = new QMenu( i18n( "Super Categories" ) );
    menu->addMenu( members );
    QAction* newCategoryAction = 0;
    if ( item ) {
        QStringList grps = memberMap.groups( _category->name() );

        for( QStringList::ConstIterator it = grps.constBegin(); it != grps.constEnd(); ++it ) {
            if (!memberMap.canAddMemberToGroup(_category->name(), *it, item->text(0)))
                continue;
            QAction* action = members->addAction( *it );
            action->setCheckable(true);
            action->setChecked( (bool) memberMap.members( _category->name(), *it, false ).contains( item->text(0) ) );
            action->setData( *it );
        }

        if ( !grps.isEmpty() )
            members->addSeparator();
        newCategoryAction = members->addAction( i18n("New Category..." ) );
    }

    QAction* newSubcategoryAction = menu->addAction( i18n( "Create Subcategory..." ) );

    // -------------------------------------------------- Take item out of category
    Q3ListViewItem* parent = item ? item->parent() : 0;
    QAction* takeAction = 0;
    if ( parent )
        takeAction = menu->addAction( i18n( "Take item out of category %1", parent->text(0) ) );

    // -------------------------------------------------- sort
    QLabel* sortTitle = new QLabel( i18n("<b>Sorting</b>") );
    sortTitle->setAlignment( Qt::AlignCenter );
    action = new QWidgetAction( menu );
    action->setDefaultWidget( sortTitle );
    menu->addAction( action );

    QAction* usageAction = menu->addAction( i18n("Usage") );
    QAction* alphaFlatAction = menu->addAction( i18n("Alphabetical (Flat)") );
    QAction* alphaTreeAction = menu->addAction( i18n("Alphabetical (Tree)") );
    usageAction->setCheckable(true);
    usageAction->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortLastUse);
    alphaFlatAction->setCheckable(true);
    alphaFlatAction->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaFlat);
    alphaTreeAction->setCheckable(true);
    alphaTreeAction->setChecked( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree);

    if ( !item ) {
        deleteAction->setEnabled( false );
        renameAction->setEnabled( false );
        members->setEnabled( false );
        newSubcategoryAction->setEnabled( false );
    }
// -------------------------------------------------- exec
    QAction* which = menu->exec( pos );
    if ( which == 0 )
        return;

    else if ( which == deleteAction ) {
        int code = KMessageBox::warningContinueCancel( this, i18n("<p>Do you really want to delete \"%1\"?<br/>"
                                                                  "Deleting the item will remove any information "
                                                                  "about it from any image containing the item.</p>"
                                                       ,item->text(0)),
                                                       i18n("Really Delete %1?",item->text(0)),
                                                       KGuiItem(i18n("&Delete"),QString::fromLatin1("editdelete")) );
        if ( code == KMessageBox::Continue ) {
            _category->removeItem( item->text(0) );
            rePopulate();
        }
    }
    else if ( which == renameAction ) {
        bool ok;
        QString newStr = KInputDialog::getText( i18n("Rename Item"), i18n("Enter new name:"),
                                                item->text(0), &ok, this );

        if ( ok && newStr != item->text(0) ) {
            int code = KMessageBox::questionYesNo( this, i18n("<p>Do you really want to rename \"%1\" to \"%2\"?<br/>"
                                                              "Doing so will rename \"%3\" "
                                                              "on any image containing it.</p>"
                                               ,item->text(0),newStr,item->text(0)),
                                               i18n("Really Rename %1?",item->text(0)) );
            if ( code == KMessageBox::Yes ) {
                QString oldStr = item->text(0);
                _category->renameItem( oldStr, newStr );
                bool checked = static_cast<Q3CheckListItem*>(item)->isOn();
                rePopulate();
                // rePopuldate doesn't ask the backend if the item should be checked, so we need to do that.
                checkItem( newStr, checked );

                // rename the category image too
                QString oldFile = _category->fileForCategoryImage( category(), oldStr );
                QString newFile = _category->fileForCategoryImage( category(), newStr );
                KIO::move( KUrl(oldFile), KUrl(newFile) );
            }
        }
    }
    else if ( which == usageAction ) {
        Settings::SettingsData::instance()->setViewSortType( Settings::SortLastUse );
    }
    else if ( which == alphaTreeAction ) {
        Settings::SettingsData::instance()->setViewSortType( Settings::SortAlphaTree );
    }
    else if ( which == alphaFlatAction ) {
        Settings::SettingsData::instance()->setViewSortType( Settings::SortAlphaFlat );
    }
    else if ( which == newCategoryAction ) {
        QString superCategory = KInputDialog::getText( i18n("New Super Category"), i18n("New Super Category Name:") );
        if ( superCategory.isEmpty() )
            return;
        memberMap.addGroup( _category->name(), superCategory );
        memberMap.addMemberToGroup( _category->name(), superCategory, item->text(0) );
        //DB::ImageDB::instance()->setMemberMap( memberMap );
        rePopulate();
    }
    else if ( which == newSubcategoryAction ) {
        QString subCategory = KInputDialog::getText( i18n("New Sub Category"), i18n("New Sub Category Name:") );
        if ( subCategory.isEmpty() )
            return;

         _category->addItem( subCategory );
         memberMap.addGroup( _category->name(), item->text(0) );
         memberMap.addMemberToGroup( _category->name(), item->text(0), subCategory );
         //DB::ImageDB::instance()->setMemberMap( memberMap );
        if ( isInputMode() )
            _category->addItem( subCategory );

        rePopulate();
        if ( isInputMode() )
            checkItem( subCategory, true );
    }
    else if ( which == takeAction ) {
        memberMap.removeMemberFromGroup( _category->name(), parent->text(0), item->text(0) );
        rePopulate();
    }
    else {
        QString checkedItem = which->data().value<QString>();
        if ( which->isChecked() ) // choosing the item doesn't check it, so this is the value before.
            memberMap.addMemberToGroup( _category->name(), checkedItem, item->text(0) );
        else
            memberMap.removeMemberFromGroup( _category->name(), checkedItem, item->text(0) );
        rePopulate();
    }

    delete menu;
}


void AnnotationDialog::ListSelect::addItems( DB::CategoryItem* item, Q3ListViewItem* parent )
{
     for( QList<DB::CategoryItem*>::ConstIterator subcategoryIt = item->_subcategories.constBegin(); subcategoryIt != item->_subcategories.constEnd(); ++subcategoryIt ) {
        CheckDropItem* newItem = 0;

        if ( parent == 0 )
            newItem = new CheckDropItem( _listView, (*subcategoryIt)->_name, QString() );
        else
            newItem = new CheckDropItem( _listView, parent, (*subcategoryIt)->_name, QString() );

        newItem->setOpen( true );
        configureItem( newItem );

        addItems( *subcategoryIt, newItem );
    }
}

void AnnotationDialog::ListSelect::populate()
{
    _listView->clear();

    if ( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree )
        populateAlphaTree();
    else if ( Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaFlat )
        populateAlphaFlat();
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

void AnnotationDialog::ListSelect::slotSortAlphaTree()
{
    Settings::SettingsData::instance()->setViewSortType( Settings::SortAlphaTree );
}

void AnnotationDialog::ListSelect::slotSortAlphaFlat()
{
    Settings::SettingsData::instance()->setViewSortType( Settings::SortAlphaFlat );
}

void AnnotationDialog::ListSelect::rePopulate()
{
    const int x = _listView->contentsX();
    const int y = _listView->contentsY();

    const StringSet on = itemsOn();
    const StringSet noChange = itemsUnchanged();
    populate();
    setSelection( on, noChange );

    if( ShowSelectionOnlyManager::instance().selectionIsLimited() )
        limitToSelection();

    _listView->setContentsPos( x, y );
}

void AnnotationDialog::ListSelect::showOnlyItemsMatching( const QString& text )
{
    ListViewTextMatchHider dummy( text, Settings::SettingsData::instance()->matchType(), _listView );
    ShowSelectionOnlyManager::instance().unlimitFromSelection();
}

void AnnotationDialog::ListSelect::populateAlphaTree()
{
    DB::CategoryItemPtr item = _category->itemsCategories();

    _listView->setRootIsDecorated( true );
    addItems( item.data(), 0 );
    _listView->setSorting( 0 );
}

void AnnotationDialog::ListSelect::populateAlphaFlat()
{
    QStringList items = _category->itemsInclCategories();

    _listView->setRootIsDecorated( false );
    for( QStringList::ConstIterator itemIt = items.constBegin(); itemIt != items.constEnd(); ++itemIt ) {
        CheckDropItem* item = new CheckDropItem( _listView, *itemIt, *itemIt );
        configureItem( item );
    }

    _listView->setSorting( 1 );
}

void AnnotationDialog::ListSelect::populateMRU()
{
    QStringList items = _category->itemsInclCategories();

    _listView->setRootIsDecorated( false );
    int index = 100000; // This counter will be converted to a string, and compared, and we don't want "1111" to be less than "2"
    for( QStringList::ConstIterator itemIt = items.constBegin(); itemIt != items.constEnd(); ++itemIt ) {
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
        data->setViewSortType( Settings::SortAlphaTree );
    else if ( data->viewSortType() == Settings::SortAlphaTree )
        data->setViewSortType( Settings::SortAlphaFlat );
    else
        data->setViewSortType( Settings::SortLastUse );
}

void AnnotationDialog::ListSelect::updateListview()
{
    // update item list (e.g. when MatchType changes):
    showOnlyItemsMatching( text() );
}

void AnnotationDialog::ListSelect::limitToSelection()
{
    if ( !isInputMode() )
        return;

    _showSelectedOnly->setChecked( true );
    ListViewCheckedHider dummy( _listView );
}

void AnnotationDialog::ListSelect::showAllChildren()
{
    _showSelectedOnly->setChecked( false );
    showOnlyItemsMatching( QString() );
}

void AnnotationDialog::ListSelect::updateSelectionCount()
{
    if ( _baseTitle.isEmpty()    //-> first time
            || ! parentWidget()->windowTitle().startsWith( _baseTitle ) //-> title has changed
       )
    {
        // save the original parentWidget title
        _baseTitle = parentWidget()->windowTitle();
    }
    switch( _mode )
    {
        case InputMultiImageConfigMode:
            if ( itemsUnchanged().size() > 0 )
            { // if min != max
                // tri-state selection -> show min-max (selected items vs. partially selected items):
                parentWidget()->setWindowTitle( QString::fromLatin1( "%1 (%2-%3)" )
                        .arg( _baseTitle )
                        .arg( itemsOn().size() )
                        .arg( itemsOn().size() + itemsUnchanged().size() ) );
                break;
            } // else fall through and only show one number:
        case InputSingleImageConfigMode:
            if ( itemsOn().size() > 0 )
            { // if any tags have been selected
                // "normal" on/off states -> show selected items
                parentWidget()->setWindowTitle( QString::fromLatin1( "%1 (%2)" )
                        .arg( _baseTitle )
                        .arg( itemsOn().size() ) );
                break;
            } // else fall through and only show category
        case SearchMode:
            // no indicator while searching
            parentWidget()->setWindowTitle( _baseTitle );
            break;
    }
}


void AnnotationDialog::ListSelect::configureItem( CategoryListView::CheckDropItem* item )
{
    bool isDNDAllowed = Settings::SettingsData::instance()->viewSortType() == Settings::SortAlphaTree;
    item->setDNDEnabled( isDNDAllowed && ! _category->isSpecialCategory() );
    item->setTristate( _mode == InputMultiImageConfigMode );
}

bool AnnotationDialog::ListSelect::isInputMode() const
{
    return _mode != SearchMode;
}

StringSet AnnotationDialog::ListSelect::itemsOn() const
{
    return itemsOfState( Q3CheckListItem::On );
}

StringSet AnnotationDialog::ListSelect::itemsOff() const
{
    return itemsOfState( Q3CheckListItem::Off );
}

StringSet AnnotationDialog::ListSelect::itemsOfState( Q3CheckListItem::ToggleState state ) const
{
    StringSet res;
    for ( Q3ListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt ) {
        if ( static_cast<Q3CheckListItem*>(*itemIt)->state() == state )
            res.insert( (*itemIt)->text(0) );
    }
    return res;
}

StringSet AnnotationDialog::ListSelect::itemsUnchanged() const
{
    return itemsOfState( Q3CheckListItem::NoChange );
}

void AnnotationDialog::ListSelect::checkItem( const QString itemText, bool b )
{
    Q3ListViewItem* item = _listView->findItem( itemText, 0 );
    if ( item )
        static_cast<Q3CheckListItem*>(item)->setOn( b );
    else
        Q_ASSERT( false );
}

/**
 * An item may be member of a number of categories. Mike may be a member of coworkers and friends.
 * Selecting the item in one subcategory, should select him in all.
 */
void AnnotationDialog::ListSelect::ensureAllInstancesAreStateChanged( Q3ListViewItem* item )
{
    bool on = static_cast<Q3CheckListItem*>(item)->isOn();
    for ( Q3ListViewItemIterator itemIt( _listView ); *itemIt; ++itemIt ) {
        if ( (*itemIt) != item && (*itemIt)->text(0) == item->text(0) )
            static_cast<Q3CheckListItem*>(*itemIt)->setOn( on );
    }
}

QWidget* AnnotationDialog::ListSelect::lineEdit()
{
    return _lineEdit;
}

#include "ListSelect.moc"

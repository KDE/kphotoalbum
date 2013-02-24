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
#include "SubCategoriesPage.h"
#include <KMessageBox>
#include <kinputdialog.h>
#include <DB/ImageDB.h>
#include <klocale.h>
#include <QPushButton>
#include <QListWidget>
#include <KComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "DB/CategoryCollection.h"

Settings::SubCategoriesPage::SubCategoriesPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* lay1 = new QVBoxLayout( this );

    // Category
    QHBoxLayout* lay2 = new QHBoxLayout;
    lay1->addLayout( lay2 );

    QLabel* label = new QLabel( i18n( "Category:" ), this );
    lay2->addWidget( label );
    _category = new KComboBox( this );
    lay2->addWidget( _category );
    lay2->addStretch(1);

    QHBoxLayout* lay3 = new QHBoxLayout;
    lay1->addLayout( lay3 );

    // Groups
    QVBoxLayout* lay4 = new QVBoxLayout;
    lay3->addLayout( lay4 );

    label = new QLabel( i18n( "Super Categories:" ), this );
    lay4->addWidget( label );
    _groups = new QListWidget( this );
    lay4->addWidget( _groups );

    // Members
    QVBoxLayout* lay5 = new QVBoxLayout;
    lay3->addLayout( lay5 );

    label = new QLabel( i18n( "Items of Category:" ), this );
    lay5->addWidget( label );
    _members = new QListWidget( this );
    lay5->addWidget( _members );

    // Buttons
    QHBoxLayout* lay6 = new QHBoxLayout;
    lay1->addLayout( lay6 );
    lay6->addStretch(1);

    QPushButton* add = new QPushButton( i18n("Add Super Category..." ), this );
    lay6->addWidget( add );
    _rename = new QPushButton( i18n( "Rename Super Category..."), this );
    lay6->addWidget( _rename );
    _del = new QPushButton( i18n("Delete Super Category" ), this );
    lay6->addWidget( _del );

    // Notice
    QLabel* notice = new QLabel( i18n("<b>Notice:</b> It is also possible to set up subcategories in the annotation dialog, simply by dragging items." ), this );
    lay1->addWidget( notice );

    // Setup the actions
    _memberMap = DB::ImageDB::instance()->memberMap();
    connect( DB::ImageDB::instance()->categoryCollection(),
             SIGNAL(itemRemoved(DB::Category*,QString)),
             &_memberMap, SLOT(deleteItem(DB::Category*,QString)) );
    connect( DB::ImageDB::instance()->categoryCollection(),
             SIGNAL(itemRenamed(DB::Category*,QString,QString)),
             &_memberMap, SLOT(renameItem(DB::Category*,QString,QString)) );
    connect( _category, SIGNAL(activated(QString)), this, SLOT(slotCategoryChanged(QString)) );
    connect( _groups, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(slotGroupSelected(QListWidgetItem*)) );
    connect( _rename, SIGNAL(clicked()), this, SLOT(slotRenameGroup()) );
    connect( add, SIGNAL(clicked()), this, SLOT(slotAddGroup()) );
    connect( _del, SIGNAL(clicked()), this, SLOT(slotDelGroup()) );

    _members->setSelectionMode( QAbstractItemView::MultiSelection );
}

/**
   When the user selects a new category from the combo box then this method is called
   Its purpose is too fill the groups and members listboxes.
*/
void Settings::SubCategoriesPage::slotCategoryChanged( const QString& text)
{
    slotCategoryChanged( DB::ImageDB::instance()->categoryCollection()->nameForText(text), true );
}

void Settings::SubCategoriesPage::slotCategoryChanged( const QString& name, bool saveGroups )
{
    if ( saveGroups ) {
        // We do not want to save groups when renaming categories
        saveOldGroup();
    }

    _groups->blockSignals(true);
    _groups->clear();
    _groups->blockSignals(false);

    _currentCategory = name;
    if (name.isNull())
        return;
    QStringList groupList = _memberMap.groups( name );

    _groups->blockSignals(true);
    _groups->addItems( groupList );
    _groups->blockSignals(false);

    _members->clear();
    QStringList list = DB::ImageDB::instance()->categoryCollection()->categoryForName(name)->items();
    list += _memberMap.groups( name );
    QStringList uniq;
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( !uniq.contains(*it) )
            uniq << *it;
    }

    uniq.sort();
    _members->addItems( uniq );

    _currentGroup.clear();

    _members->clearSelection();
    _members->setEnabled(false);

    setButtonStates();
}

void Settings::SubCategoriesPage::slotGroupSelected( QListWidgetItem* item )
{
    saveOldGroup();
    if ( item )
        selectMembers( item->text() );
}

void Settings::SubCategoriesPage::slotAddGroup()
{
    bool ok;
    QString text = KInputDialog::getText( i18n( "New Group" ), i18n("Group name:"), QString(), &ok );
    if ( ok ) {
        saveOldGroup();
        DB::ImageDB::instance()->categoryCollection()->categoryForName( _currentCategory )->addItem( text );
        _memberMap.addGroup(_currentCategory, text);
        slotCategoryChanged( _currentCategory, false );
        QList<QListWidgetItem*> items = _groups->findItems(text, Qt::MatchExactly);
        Q_ASSERT(items.count() != 0);
        _groups->setCurrentItem( items[0] ); // also emits currentChanged()
        // selectMembers() is called automatically by slotGroupSelected()
    }
}

void Settings::SubCategoriesPage::slotRenameGroup()
{
    Q_ASSERT( !_currentGroup.isNull() );
    bool ok;
    QString text = KInputDialog::getText( i18n( "New Group" ), i18n("Group name:"), _currentGroup, &ok );
    if ( ok ) {
        saveOldGroup();
        _memberMap.renameGroup( _currentCategory, _currentGroup, text );
        DB::ImageDB::instance()->categoryCollection()->categoryForName( _currentCategory )->renameItem( _currentGroup, text );
        slotCategoryChanged( _currentCategory, false );
        QList<QListWidgetItem*> items = _groups->findItems(text, Qt::MatchExactly);
        Q_ASSERT(!items.empty());
        _groups->setCurrentItem( items[0] );
    }
}

void Settings::SubCategoriesPage::slotDelGroup()
{
    Q_ASSERT( !_currentGroup.isNull() );
    int res = KMessageBox::warningContinueCancel( this, i18n( "Really delete group %1?" ,_currentGroup ),i18n("Delete Group"),KGuiItem(i18n("&Delete"),QString::fromLatin1("editdelete")) );
    if ( res == KMessageBox::Cancel )
        return;

    _memberMap.deleteGroup( _currentCategory, _currentGroup );
    DB::ImageDB::instance()->categoryCollection()->categoryForName( _currentCategory )->removeItem( _currentGroup );
    _currentGroup.clear();
    slotCategoryChanged( _currentCategory, false );
}

void Settings::SubCategoriesPage::saveOldGroup()
{
    if ( _currentCategory.isNull() || _currentGroup.isNull() )
        return;

    QStringList list;
    for ( int i=0; i < _members->count(); ++i ) {
        QListWidgetItem* item = _members->item(i);
        if ( item->isSelected() )
            list << item->text();
    }

    _memberMap.setMembers(_currentCategory, _currentGroup, list);
}

void Settings::SubCategoriesPage::selectMembers( const QString& group )
{
    _currentGroup = group;

    QStringList list = _memberMap.members(_currentCategory,group, false );

    for ( int i=0; i < _members->count(); ++i ) {
        QListWidgetItem* item = _members->item(i);
        if (!_memberMap.canAddMemberToGroup(_currentCategory, group, item->text())) {
            item->setSelected(false);
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        }
        else {
            item->setFlags(item->flags() | Qt::ItemIsSelectable);
            item->setSelected(list.contains(item->text()));
        }
    }

    _members->setEnabled(true);

    setButtonStates();
}

void Settings::SubCategoriesPage::setButtonStates()
{
    bool b = !_currentGroup.isNull();
    _rename->setEnabled( b );
    _del->setEnabled( b );
}

void Settings::SubCategoriesPage::slotPageChange()
{
    _category->clear();
     QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
     for( QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        if ( !(*it)->isSpecialCategory() )
            _category->addItem( (*it)->text() );
    }

     slotCategoryChanged( _category->currentText() );
}

void Settings::SubCategoriesPage::saveSettings()
{
    saveOldGroup();
    DB::ImageDB::instance()->memberMap() = _memberMap;
}

void Settings::SubCategoriesPage::loadSettings()
{
    slotCategoryChanged( _currentCategory, false );
}

void Settings::SubCategoriesPage::categoryRenamed( const QString& oldName, const QString& newName )
{
    if ( _currentCategory == oldName )
        _currentCategory = newName;
}

DB::MemberMap* Settings::SubCategoriesPage::memberMap()
{
    return &_memberMap;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

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
#include "CategoryPage.h"
#include "UntaggedGroupBox.h"
#include <DB/ImageDB.h>
#include "SettingsDialog.h"
#include <kmessagebox.h>
#include "CategoryItem.h"
#include <KComboBox>
#include <klocale.h>
#include <kpushbutton.h>
#include <QSpinBox>
#include <kicondialog.h>
#include <KLineEdit>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "DB/CategoryCollection.h"

Settings::CategoryPage::CategoryPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* lay1 = new QVBoxLayout(this);
    QHBoxLayout* lay2 = new QHBoxLayout;
    lay1->addLayout( lay2 );

    _categories = new QListWidget( this );
    connect( _categories, SIGNAL( itemClicked( QListWidgetItem* ) ), this, SLOT( edit( QListWidgetItem* ) ) );
    lay2->addWidget( _categories );


    QGridLayout* lay3 = new QGridLayout;
    lay2->addLayout( lay3 );

    _labelLabel = new QLabel( i18n( "Label:" ), this );
    lay3->addWidget( _labelLabel, 0, 0 );

    _text = new KLineEdit( this );
    connect( _text, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( slotLabelChanged( const QString& ) ) );

    lay3->addWidget( _text, 0, 1 );


    // Icon
    _iconLabel = new QLabel( i18n("Icon:" ), this );
    lay3->addWidget( _iconLabel, 1, 0 );

    _icon = new KIconButton(  this );
    lay3->addWidget( _icon, 1, 1 );
    _icon->setIconSize(32);
    _icon->setIcon( QString::fromLatin1( "personsIcon" ) );
    connect( _icon, SIGNAL( iconChanged( QString ) ), this, SLOT( slotIconChanged( QString ) ) );


    // Thumbnail size
    _thumbnailSizeInCategoryLabel = new QLabel( i18n( "Thumbnail Size: " ), this );
    lay3->addWidget( _thumbnailSizeInCategoryLabel, 2, 0 );

    _thumbnailSizeInCategory = new QSpinBox;
    _thumbnailSizeInCategory->setRange( 32, 512 );
    _thumbnailSizeInCategory->setSingleStep( 32 );
    lay3->addWidget( _thumbnailSizeInCategory, 2, 1 );
    connect( _thumbnailSizeInCategory, SIGNAL( valueChanged( int ) ), this, SLOT( thumbnailSizeChanged( int ) ) );


    // Preferred View
    _preferredViewLabel = new QLabel( i18n("Preferred view:"), this );
    lay3->addWidget( _preferredViewLabel, 3, 0 );

    _preferredView = new KComboBox( this );
    lay3->addWidget( _preferredView, 3, 1 );
    QStringList list;
    list << i18n("List View") << i18n("List View with Custom Thumbnails") << i18n("Icon View") << i18n("Icon View with Custom Thumbnails");
    _preferredView->addItems( list );
    connect( _preferredView, SIGNAL( activated( int ) ), this, SLOT( slotPreferredViewChanged( int ) ) );

    QHBoxLayout* lay4 = new QHBoxLayout;
    lay1->addLayout( lay4 );

    KPushButton* newItem = new KPushButton( i18n("New"), this );
    connect( newItem, SIGNAL( clicked() ), this, SLOT( slotNewItem() ) );

    _delItem = new KPushButton( i18n("Delete"), this );
    connect( _delItem, SIGNAL( clicked() ), this, SLOT( slotDeleteCurrent() ) );

    lay4->addStretch(1);
    lay4->addWidget( newItem );
    lay4->addWidget( _delItem );

    _current = 0;

    // Untagged images
    _untaggedBox = new UntaggedGroupBox(this);
    lay1->addWidget(_untaggedBox);

}

void Settings::CategoryPage::edit( QListWidgetItem* i )
{
    if ( i == 0 )
        return;

    Settings::CategoryItem* item = static_cast<Settings::CategoryItem*>(i);
    _current = item;
    _text->setText( item->text() );
    _icon->setIcon( item->icon() );
    _thumbnailSizeInCategory->setValue( item->thumbnailSize() );
    _preferredView->setCurrentIndex( static_cast<int>(item->viewType()) );
    enableDisable( true );
}

void Settings::CategoryPage::slotLabelChanged( const QString& label )
{
    if( _current ) {
        emit currentCategoryNameChanged( _current->text(), label );
        _current->setLabel( label );
    }
}

void Settings::CategoryPage::slotIconChanged( const QString& icon )
{
    if( _current )
        _current->setIcon( icon );
}

void Settings::CategoryPage::thumbnailSizeChanged( int size )
{
    if ( _current )
        _current->setThumbnailSize( size );
}

void Settings::CategoryPage::slotPreferredViewChanged( int i )
{
    if ( _current ) {
        _current->setViewType( static_cast<DB::Category::ViewType>(i) );
    }
}

void Settings::CategoryPage::slotNewItem()
{
    _current = new Settings::CategoryItem( QString(), QString(), QString(), DB::Category::TreeView, 64, _categories );
    _text->setText( QString::fromLatin1( "" ) );
    _icon->setIcon( QIcon() );
    _thumbnailSizeInCategory->setValue( 64 );
    enableDisable( true );
    _current->setSelected( true );
    _text->setFocus();
}

void Settings::CategoryPage::slotDeleteCurrent()
{
    int answer = KMessageBox::questionYesNo( this, i18n("<p>Really delete category '%1'?</p>", _current->text()) );
    if ( answer == KMessageBox::No )
        return;

    _deleted.append( _current );
    _categories->takeItem( _categories->row(_current) );
    _current = 0;
    _text->setText( QString::fromLatin1( "" ) );
    _icon->setIcon( QIcon() );
    _thumbnailSizeInCategory->setValue(64);
    enableDisable(false);
}

void Settings::CategoryPage::enableDisable( bool b )
{
    _delItem->setEnabled( b );
    _labelLabel->setEnabled( b );
    _text->setEnabled( b );
    _icon->setEnabled( b );
    _iconLabel->setEnabled( b );
    _thumbnailSizeInCategoryLabel->setEnabled( b );
    _thumbnailSizeInCategory->setEnabled( b );
    _preferredViewLabel->setEnabled( b );
    _preferredView->setEnabled( b );
}

void Settings::CategoryPage::saveSettings( Settings::SettingsData* opt, DB::MemberMap* memberMap )
{
    // Delete items
    for( QList<CategoryItem*>::Iterator it = _deleted.begin(); it != _deleted.end(); ++it ) {
        (*it)->removeFromDatabase();
    }

    // Created or Modified items
    for (int i =0; i<_categories->count();++i) {
        CategoryItem* item = static_cast<CategoryItem*>( _categories->item(i) );
        item->submit( memberMap );
    }

    _untaggedBox->saveSettings( opt );
}

void Settings::CategoryPage::loadSettings( Settings::SettingsData* opt )
{
    _categories->clear();
    QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    for( QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        if( !(*it)->isSpecialCategory() ) {
            new CategoryItem( (*it)->name(), (*it)->text(),(*it)->iconName(),(*it)->viewType(), (*it)->thumbnailSize(), _categories );
        }
    }

    _untaggedBox->loadSettings( opt );
}




// vi:expandtab:tabstop=4 shiftwidth=4:

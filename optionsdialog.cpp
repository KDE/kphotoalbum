/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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

#include "optionsdialog.h"
#include <kfiledialog.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <qspinbox.h>
#include "options.h"
#include <kicondialog.h>
#include <qlistbox.h>
#include <kmessagebox.h>
#include "imagedb.h"
#include <qcheckbox.h>
#include <kinputdialog.h>

OptionsDialog::OptionsDialog( QWidget* parent, const char* name )
    :KDialogBase( Tabbed, i18n( "Options" ), Ok | Cancel, Ok, parent, name )
{
    createGeneralPage();
    createOptionGroupsPage();
    createGroupConfig();
}

void OptionsDialog::createGeneralPage()
{
    QWidget* top = addPage( i18n("General" ) );
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    // Thumbnail size
    QLabel* label = new QLabel( i18n("Thumbnail Size"), top );
    _thumbnailSize = new QSpinBox( 16, 512, 8, top );
    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );
    lay2->addWidget( label );
    lay2->addWidget( _thumbnailSize );
    lay2->addStretch(1);

    // Thrust time stamps
    label = new QLabel( i18n("Trust time stamps on new files"), top );
    _trustTimeStamps = new KComboBox( top );
    _trustTimeStamps->insertStringList( QStringList() << i18n("Always") << i18n("Ask") << i18n("Never") );
    QHBoxLayout* lay3 = new QHBoxLayout( lay1, 6 );
    lay3->addWidget( label );
    lay3->addWidget( _trustTimeStamps );
    lay3->addStretch( 1 );

    // Max images to show per page
    label = new QLabel( i18n("Maximum images to show per page"), top );
    _maxImages = new QSpinBox( 10, 10000, 1, top ) ;
    QHBoxLayout* lay4 = new QHBoxLayout( lay1, 6 );
    lay4->addWidget( label );
    lay4->addWidget( _maxImages );
    lay4->addStretch(1);

    // Auto save
    label = new QLabel( i18n("Auto save every: "), top );
    _autosave = new QSpinBox( 1, 120, 1, top );
    _autosave->setSuffix( i18n( "min." ) );
    QHBoxLayout* lay6 = new QHBoxLayout( lay1, 6 );
    lay6->addWidget( label );
    lay6->addWidget( _autosave );
    lay6->addStretch( 1 );

    // Viewer Size
    QHBoxLayout* lay7 = new QHBoxLayout( lay1, 6 );
    label = new QLabel( i18n("Viewer Size: "), top );
    lay7->addWidget( label );

    _width = new QSpinBox( 100, 5000, 50, top );
    lay7->addWidget( _width );

    label = new QLabel( QString::fromLatin1("x"), top );
    lay7->addWidget( label );

    _height = new QSpinBox( 100, 5000, 50, top );
    lay7->addWidget( _height );

    lay7->addStretch(1);


    connect( this, SIGNAL( okClicked() ), this, SLOT( slotMyOK() ) );
    lay1->addStretch(1);
}

class OptionGroupItem :public QListBoxText
{
public:
    OptionGroupItem( const QString& optionGroup, const QString& text, const QString& icon,
                     QListBox* parent );
    void setLabel( const QString& label );

    QString _optionGroupOrig, _textOrig, _iconOrig;
    QString _text, _icon;
};

OptionGroupItem::OptionGroupItem( const QString& optionGroup, const QString& text, const QString& icon,
                                  QListBox* parent )
    :QListBoxText( parent, text ),
     _optionGroupOrig( optionGroup ), _textOrig( text ), _iconOrig( icon ),
     _text( text ), _icon( icon )
{
}

void OptionGroupItem::setLabel( const QString& label )
{
    setText( label );
    _text = label;

    // unfortunately setText do not call updateItem, so we need to force it with this hack.
    bool b = listBox()->isSelected( this );
    listBox()->setSelected( this, !b );
    listBox()->setSelected( this, b );
}


void OptionsDialog::createOptionGroupsPage()
{
    QWidget* top = addPage( i18n("Option Groups") );

    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );
    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );

    _optionGroups = new QListBox( top );
    connect( _optionGroups, SIGNAL( clicked( QListBoxItem* ) ), this, SLOT( edit( QListBoxItem* ) ) );
    lay2->addWidget( _optionGroups );

    QVBoxLayout* lay3 = new QVBoxLayout( lay2, 6 );
    QLabel* label = new QLabel( i18n( "Label:" ), top );
    lay3->addWidget( label );

    _text = new QLineEdit( top );
    connect( _text, SIGNAL( textChanged( const QString& ) ),
             this, SLOT( slotLabelChanged( const QString& ) ) );

    lay3->addWidget( _text );

    _icon = new KIconButton(  top );
    QHBoxLayout* lay5 = new QHBoxLayout( lay3 );
    lay5->addWidget( _icon );
    lay5->addStretch(1);
    _icon->setIconSize(32);
    _icon->setIcon( QString::fromLatin1( "personsIcon" ) );
    connect( _icon, SIGNAL( iconChanged( QString ) ), this, SLOT( slotIconChanged( QString ) ) );
    lay3->addStretch(1);

    QHBoxLayout* lay4 = new QHBoxLayout( lay1, 6 );
    KPushButton* newItem = new KPushButton( i18n("New"), top );
    connect( newItem, SIGNAL( clicked() ), this, SLOT( slotNewItem() ) );

    _delItem = new KPushButton( i18n("Delete"), top );
    connect( _delItem, SIGNAL( clicked() ), this, SLOT( slotDeleteCurrent() ) );

    lay4->addStretch(1);
    lay4->addWidget( newItem );
    lay4->addWidget( _delItem );

    _current = 0;
}



void OptionsDialog::show()
{
    Options* opt = Options::instance();

    // General page
    _thumbnailSize->setValue( opt->thumbSize() );
    _trustTimeStamps->setCurrentItem( opt->tTimeStamps() );
    _autosave->setValue( opt->autoSave() );
    _maxImages->setValue( opt->maxImages() );
    _width->setValue( opt->viewerSize().width() );
    _height->setValue( opt->viewerSize().height() );

    // Config Groups page
    _optionGroups->clear();
    QStringList grps = opt->optionGroups();
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        new OptionGroupItem( *it, opt->textForOptionGroup( *it ), opt->iconNameForOptionGroup( *it ),
                             _optionGroups );
    }
    enableDisable( false );
    KDialogBase::show();
}



// KDialogBase has a slotOK which we do not want to override.
void OptionsDialog::slotMyOK()
{
    Options* opt = Options::instance();

    // General
    opt->setThumbSize( _thumbnailSize->value() );
    opt->setTTimeStamps( (Options::TimeStampTrust) _trustTimeStamps->currentItem() );
    opt->setAutoSave( _autosave->value() );
    opt->setMaxImages( _maxImages->value() );
    opt->setViewerSize( _width->value(), _height->value() );

    // ----------------------------------------------------------------------
    // Option Groups

    // Delete items
    for( QValueList<OptionGroupItem*>::Iterator it = _deleted.begin(); it != _deleted.end(); ++it ) {
        if ( !(*it)->_optionGroupOrig.isNull() ) {
            // the Options instance knows about the item.
            opt->deleteOptionGroup( (*it)->_optionGroupOrig );
        }
    }

    // Created or Modified items
    for ( QListBoxItem* i = _optionGroups->firstItem(); i; i = i->next() ) {
        OptionGroupItem* item = static_cast<OptionGroupItem*>( i );
        if ( item->_optionGroupOrig.isNull() ) {
            // New Item
            opt->addOptionGroup( item->_text, item->_text, item->_icon );
        }
        else {
            if ( item->_text != item->_textOrig ) {
                opt->renameOptionGroup( item->_optionGroupOrig, item->_text );
                item->_optionGroupOrig =item->_text;
            }
            if ( item->_icon != item->_iconOrig ) {
                opt->setIconForOptionGroup( item->_optionGroupOrig, item->_icon );
            }
        }
    }

    saveOldGroup();
    opt->setMemberMap( _memberMap );

    // misc stuff
    emit changed();
}


void OptionsDialog::edit( QListBoxItem* i )
{
    if ( i == 0 )
        return;

    OptionGroupItem* item = static_cast<OptionGroupItem*>(i);
    _current = item;
    _text->setText( item->_text );
    _icon->setIcon( item->_icon );
    enableDisable( true );
}

void OptionsDialog::slotLabelChanged( const QString& label)
{
    if( _current )
        _current->setLabel( label );
}

void OptionsDialog::slotIconChanged( QString icon )
{
    if( _current )
        _current->_icon = icon;
}

void OptionsDialog::slotNewItem()
{
    _current = new OptionGroupItem( QString::null, QString::null, QString::null, _optionGroups );
    _text->setText( QString::fromLatin1( "" ) );
    _icon->setIcon( QString::null );
    enableDisable( true );
    _optionGroups->setSelected( _current, true );
    _text->setFocus();
}

void OptionsDialog::slotDeleteCurrent()
{
    int count = ImageDB::instance()->countItemsOfOptionGroup( _current->_optionGroupOrig );
    int answer = KMessageBox::Yes;
    if ( count != 0 )
        KMessageBox::questionYesNo( this,
                                    i18n("<qt>Really delete group '%1'?<br>"
                                         "%2 images contains information in that group")
                                    .arg( _current->_text).arg(count) );
    if ( answer == KMessageBox::No )
        return;

    _deleted.append( _current );
    _optionGroups->takeItem( _current );
    _current = 0;
    _text->setText( QString::fromLatin1( "" ) );
    _icon->setIcon( QString::null );
    enableDisable(false);
}

void OptionsDialog::enableDisable( bool b )
{
    _delItem->setEnabled( b );
    _text->setEnabled( b );
    _icon->setEnabled( b );
}













void OptionsDialog::createGroupConfig()
{
    Options* opt = Options::instance();

    QWidget* top = addPage( i18n("Member Groups" ) );
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );

    // Category
    QHBoxLayout* lay2 = new QHBoxLayout( lay1, 6 );
    QLabel* label = new QLabel( i18n( "Category" ), top );
    lay2->addWidget( label );
    _category = new QComboBox( top );
    lay2->addWidget( _category );
    lay2->addStretch(1);

    QHBoxLayout* lay3 = new QHBoxLayout( lay1, 6 );

    // Groups
    QVBoxLayout* lay4 = new QVBoxLayout( lay3, 6 );
    label = new QLabel( i18n( "Groups:" ), top );
    lay4->addWidget( label );
    _groups = new QListBox( top );
    lay4->addWidget( _groups );

    // Members
    QVBoxLayout* lay5 = new QVBoxLayout( lay3, 6 );
    label = new QLabel( i18n( "Members:" ), top );
    lay5->addWidget( label );
    _members = new QListBox( top );
    lay5->addWidget( _members );

    // Buttons
    QHBoxLayout* lay6 = new QHBoxLayout( lay1, 6 );
    lay6->addStretch(1);

    QPushButton* rename = new QPushButton( i18n( "Rename Group"), top );
    lay6->addWidget( rename );
    QPushButton* add = new QPushButton( i18n("Add Group" ), top );
    lay6->addWidget( add );
    QPushButton* del = new QPushButton( i18n("Delete Group" ), top );
    lay6->addWidget( del );

    // Setup the actions
    _memberMap = opt->memberMap();
    connect( _category, SIGNAL( activated( const QString& ) ), this, SLOT( slotCategoryChanged( const QString& ) ) );
    connect( _groups, SIGNAL( clicked( QListBoxItem* ) ), this, SLOT( slotGroupSelected( QListBoxItem* ) ) );
    connect( rename, SIGNAL( clicked() ), this, SLOT( slotRenameGroup() ) );
    connect( add, SIGNAL( clicked() ), this, SLOT( slotAddGroup() ) );
    connect( del, SIGNAL( clicked() ), this, SLOT( slotDelGroup() ) );

    _members->setSelectionMode( QListBox::Multi );
    _category->insertStringList( opt->optionGroups() );
    slotCategoryChanged( _category->currentText() );

}

void OptionsDialog::slotCategoryChanged( const QString& name )
{
    saveOldGroup();

    _groups->clear();
    _currentCategory = name;
    _groups->insertStringList( _memberMap.groups( name ) );

    _members->clear();
    QStringList list = Options::instance()->optionValue(name);
    list += _memberMap.groups( name );
    list.sort();
    _members->insertStringList( list );
    _groups->setSelected( 0, true );

    selectMembers( _groups->text(0) );
}

void OptionsDialog::slotGroupSelected( QListBoxItem* item )
{
    saveOldGroup();
    if ( item )
        selectMembers( item->text() );
}

void OptionsDialog::slotAddGroup()
{
    bool ok;
    QString text = KInputDialog::getText( i18n( "New Group" ), i18n("Group Name"), QString::null, &ok );
    if ( ok ) {
        saveOldGroup();
        QListBoxItem* item = new QListBoxText( _groups, text );
        _groups->setCurrentItem( item );
        selectMembers( text );
    }
}

void OptionsDialog::slotRenameGroup()
{
    bool ok;
    QListBoxItem* item = _groups->item( _groups->currentItem() );
    QString currentValue = item->text();
    QString text = KInputDialog::getText( i18n( "New Group" ), i18n("Group Name"), currentValue, &ok );
    if ( ok ) {
        // PENDING(blackie) IMPLEMENT
    }
}

void OptionsDialog::slotDelGroup()
{
    if ( !_currentGroup )
        return;
    int res = KMessageBox::warningYesNo( this, i18n( "Really delete group %1" ).arg( _currentGroup ) );
    if ( res == KMessageBox::No )
        return;

    QListBoxItem* item = _groups->findItem( _currentGroup );
    delete item;
    _memberMap.deleteGroup( _currentCategory, _currentGroup );
    _currentGroup = _groups->text(0);
    selectMembers( _currentGroup );
}

void OptionsDialog::saveOldGroup()
{
    QStringList list;
    for( QListBoxItem* item = _members->firstItem(); item; item = item->next() ) {
        if ( item->isSelected() )
            list << item->text();
    }

    _memberMap.setMembers(_currentCategory, _currentGroup, list);
}

void OptionsDialog::selectMembers( const QString& group )
{
    _currentGroup = group;
    QStringList list = _memberMap.members(_currentCategory,group);
    for( QListBoxItem* item = _members->firstItem(); item; item = item->next() ) {
        _members->setSelected( item, list.contains( item->text() ) );
    }
}


#include "optionsdialog.moc"

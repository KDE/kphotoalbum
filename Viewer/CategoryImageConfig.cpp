/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#include "CategoryImageConfig.h"
#include <qlabel.h>
#include <qlayout.h>
#include <QPixmap>
#include <QGridLayout>
#include <QVBoxLayout>
#include <Q3ValueList>
#include <klocale.h>
#include <qcombobox.h>
#include "Settings/SettingsData.h"
#include "DB/CategoryCollection.h"
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include "DB/MemberMap.h"
#include "Utilities/Util.h"

CategoryImageConfig* CategoryImageConfig::_instance = 0;

CategoryImageConfig::CategoryImageConfig()
    : _image( QImage() )
{
    setWindowTitle( i18n("Configure Category Image") );
    setButtons( User1 | Close );
    setButtonText( User1, i18n("Set") );

    QWidget* top = new QWidget;
    setMainWidget( top );

    QVBoxLayout* lay1 = new QVBoxLayout( top );
    QGridLayout* lay2 = new QGridLayout;
    lay1->addLayout( lay2 );

    // Group
    QLabel* label = new QLabel( i18n("Group:" ), top );
    lay2->addWidget( label, 0, 0 );
    _group = new QComboBox( top );
    lay2->addWidget( _group, 0, 1 );
    connect( _group, SIGNAL( activated( int ) ), this, SLOT( groupChanged() ) );

    // Member
    label = new QLabel( i18n( "Member:" ), top );
    lay2->addWidget( label, 1, 0 );
    _member = new QComboBox( top );
    lay2->addWidget( _member, 1, 1 );
    connect( _member, SIGNAL( activated( int ) ), this, SLOT( memberChanged() ) );

    // Current Value
    QGridLayout* lay3 = new QGridLayout;
    lay1->addLayout( lay3 );
    label = new QLabel( i18n("Current image:"), top );
    lay3->addWidget( label, 0, 0 );

    _current = new QLabel( top );
    _current->setFixedSize( 128, 128 );
    lay3->addWidget( _current, 0, 1 );

    // New Value
    _imageLabel = new QLabel( i18n( "New image:"), top );
    lay3->addWidget( _imageLabel, 1, 0 );

    _imageLabel = new QLabel( top );
    _imageLabel->setFixedSize( 128, 128 );
    lay3->addWidget( _imageLabel, 1, 1 );

    connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotSet() ) );
}

// PENDING(blackie) convert this code to using StringSet instead.
void CategoryImageConfig::groupChanged()
{
    QString currentText = _member->currentText();
    _member->clear();
    QStringList directMembers = _info->itemsOfCategory( currentGroup() ).toList();

    QStringList list = directMembers;
    QMap<QString,QStringList> map = DB::ImageDB::instance()->memberMap().inverseMap( currentGroup() );
    for( QStringList::ConstIterator directMembersIt = directMembers.begin();
         directMembersIt != directMembers.end(); ++directMembersIt ) {
        list += map[*directMembersIt];
    }

    list = Utilities::removeDuplicates( list );

    list.sort();
    _member->addItems( list );
    int index = list.indexOf( currentText );
    if ( index != -1 )
        _member->setCurrentIndex( index );

    memberChanged();
}

void CategoryImageConfig::memberChanged()
{
    QPixmap pix = Settings::SettingsData::instance()->categoryImage( currentGroup(), _member->currentText(), 128 );
    _current->setPixmap( pix );
}

void CategoryImageConfig::slotSet()
{
    Settings::SettingsData::instance()->setCategoryImage( currentGroup(), _member->currentText(), _image );
    memberChanged();
}

QString CategoryImageConfig::currentGroup()
{
    int index = _group->currentIndex();
    return DB::ImageDB::instance()->categoryCollection()->categoryNames()[index];
}

void CategoryImageConfig::setCurrentImage( const QImage& image, const DB::ImageInfoPtr& info )
{
    _image = image;
    _imageLabel->setPixmap( QPixmap::fromImage(image) );
    _info = info;
    groupChanged();
}

CategoryImageConfig* CategoryImageConfig::instance()
{
    if ( !_instance )
        _instance = new CategoryImageConfig();
    return _instance;
}

void CategoryImageConfig::show()
{
    QString currentCategory = _group->currentText();
    _group->clear();
    Q3ValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
    int index = 0;
    int currentIndex = -1;
    for ( Q3ValueList<DB::CategoryPtr>::ConstIterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        if ( !(*categoryIt)->isSpecialCategory() ) {
            _group->addItem( (*categoryIt)->text() );
            if ( (*categoryIt)->text() == currentCategory )
                currentIndex = index;
            ++index;
        }
    }

    if ( currentIndex != -1 )
        _group->setCurrentIndex( currentIndex );
    groupChanged();


    KDialog::show();
}

#include "CategoryImageConfig.moc"

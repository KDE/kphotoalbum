#include "categoryimageconfig.h"
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <qcombobox.h>
#include "options.h"
#include "membermap.h"

CategoryImageConfig* CategoryImageConfig::_instance = 0;

CategoryImageConfig::CategoryImageConfig()
    :KDialogBase( Plain, i18n("Configure Category Image"), User1 | Close, Close, 0, "", false, false, i18n("Set") ),
     _image( QImage() )
{
    QWidget* top = plainPage();
    QVBoxLayout* lay1 = new QVBoxLayout( top, 6 );
    QGridLayout* lay2 = new QGridLayout( lay1, 2, 2 );

    // Group
    QLabel* label = new QLabel( i18n("Group" ), top );
    lay2->addWidget( label, 0, 0 );
    _group = new QComboBox( top );
    lay2->addWidget( _group, 0, 1 );
    connect( _group, SIGNAL( activated( int ) ), this, SLOT( groupChanged() ) );

    // Member
    label = new QLabel( i18n( "Member" ), top );
    lay2->addWidget( label, 1, 0 );
    _member = new QComboBox( top );
    lay2->addWidget( _member, 1, 1 );
    connect( _member, SIGNAL( activated( int ) ), this, SLOT( memberChanged() ) );

    QStringList list = Options::instance()->optionGroups();
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        _group->insertItem( Options::instance()->textForOptionGroup( *it ) );
    }

    // Current Value
    QGridLayout* lay3 = new QGridLayout( lay1, 2, 2 );
    label = new QLabel( i18n("Current Image:"), top );
    lay3->addWidget( label, 0, 0 );

    _current = new QLabel( top );
    _current->setFixedSize( 128, 128 );
    lay3->addWidget( _current, 0, 1 );

    // New Value
    _imageLabel = new QLabel( i18n( "New Image:"), top );
    lay3->addWidget( _imageLabel, 1, 0 );

    _imageLabel = new QLabel( top );
    _imageLabel->setFixedSize( 128, 128 );
    lay3->addWidget( _imageLabel, 1, 1 );

    groupChanged();
    connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotSet() ) );

}

void CategoryImageConfig::groupChanged()
{
    _member->clear();
    QStringList list = Options::instance()->optionValue( currentGroup() );
    list += Options::instance()->memberMap().groups( currentGroup() );
    list.sort();
    _member->insertStringList( list );
    memberChanged();
}

void CategoryImageConfig::memberChanged()
{
    QImage img = Options::instance()->optionImage( currentGroup(), _member->currentText(), 128 );
    _current->setPixmap( img );
}

void CategoryImageConfig::slotSet()
{
    Options::instance()->setOptionImage( currentGroup(), _member->currentText(), _image );
    memberChanged();
}

QString CategoryImageConfig::currentGroup()
{
    int index = _group->currentItem();
    return Options::instance()->optionGroups()[index];
}

void CategoryImageConfig::setCurrentImage( const QImage& image )
{
    _image = image;
    _imageLabel->setPixmap( image );
}

CategoryImageConfig* CategoryImageConfig::instance()
{
    if ( !_instance )
        _instance = new CategoryImageConfig();
    return _instance;
}

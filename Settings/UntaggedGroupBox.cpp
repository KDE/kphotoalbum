#include "UntaggedGroupBox.h"
#include "SettingsData.h"
#include <QDebug>
#include <klocale.h>
#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <DB/ImageDB.h>
#include "DB/CategoryCollection.h"

Settings::UntaggedGroupBox::UntaggedGroupBox( QWidget* parent )
    : QGroupBox( i18n("Untagged Images"), parent )
{


    QGridLayout* grid = new QGridLayout(this);
    int row = -1;

    QLabel* label = new QLabel( i18n("Category:" ) );
    grid->addWidget( label, ++row, 0 );

    _category = new QComboBox;
    grid->addWidget( _category, row, 1 );
    connect( _category, SIGNAL( currentIndexChanged( int ) ), this, SLOT( populateTagsCombo() ) );

    label = new QLabel( i18n("Tag:") );
    grid->addWidget( label, ++row, 0 );

    _tag = new QComboBox;
    grid->addWidget( _tag, row, 1 );
    _tag->setEditable(true);

    grid->setColumnStretch(1,1);
}

void Settings::UntaggedGroupBox::populateCategoryComboBox()
{
    _category->clear();
    _category->addItem( i18n("None Selected") );
    Q_FOREACH( DB::CategoryPtr category, DB::ImageDB::instance()->categoryCollection()->categories() ) {
        if (!category->isSpecialCategory() )
            _category->addItem( category->text(), category->name() );
    }
}

void Settings::UntaggedGroupBox::populateTagsCombo()
{
    _tag->clear();
     const QString currentCategory = _category->itemData(_category->currentIndex() ).value<QString>();
    if ( currentCategory.isEmpty() )
        _tag->setEnabled(false);
    else {
        _tag->setEnabled(true);
        const QStringList items = DB::ImageDB::instance()->categoryCollection()->categoryForName( currentCategory )->items();
        _tag->addItems( items );
    }
}

void Settings::UntaggedGroupBox::loadSettings( Settings::SettingsData* opt )
{
    populateCategoryComboBox();

    const QString category = opt->untaggedCategory();
    const QString tag = opt->untaggedTag();

    int categoryIndex = _category->findData( category );
    if ( categoryIndex == -1 )
        categoryIndex = 0;

    _category->setCurrentIndex( categoryIndex );
    populateTagsCombo();

    if ( categoryIndex != 0 ) {
        int tagIndex = _tag->findText( tag );
        if ( tagIndex == -1 ) {
            _tag->addItem( tag );
            tagIndex = _tag->findText( tag );
            Q_ASSERT( tagIndex != -1 );
        }
        _tag->setCurrentIndex( tagIndex );
    }


}

void Settings::UntaggedGroupBox::saveSettings( Settings::SettingsData* opt )
{
    const QString category = _category->itemData(_category->currentIndex() ).value<QString>();;
    if ( !category.isEmpty() ) {
        opt->setUntaggedCategory( category );
        opt->setUntaggedTag( _tag->currentText() );
    }
}

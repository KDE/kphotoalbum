#include "MD5CheckPage.h"
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include <QButtonGroup>
#include <QRadioButton>
#include "ImportMatcher.h"
#include <klocale.h>
#include <QLabel>
#include "DB/ImageDB.h"
#include "DB/MD5Map.h"

ImportExport::ClashInfo::ClashInfo( const QStringList& categories )
        : label(false), description(false)
{
    Q_FOREACH( const QString& category, categories )
        this->categories[category] = false;
}


bool ImportExport::MD5CheckPage::pageNeeded( const ImportSettings& settings)
{
    if ( countOfMD5Matches(settings) != 0 && clashes(settings).anyClashes() )
        return true;

    return false;
}

ImportExport::MD5CheckPage::MD5CheckPage(const ImportSettings& settings)
{
    QVBoxLayout* vlay = new QVBoxLayout( this );

    const QString txt =
        i18np("One image from the import file, has the same MD5 sum as an image in the Database, how should that be resolved?",
              "%1 images from the import file, have the same MD5 sum as images in the Database, how should that be resolved?",
              countOfMD5Matches(settings) );
    QLabel* label = new QLabel( txt );
    label->setWordWrap(true);

    vlay->addWidget( label );

    QGridLayout* grid = new QGridLayout;
    grid->setHorizontalSpacing(0);
    vlay->addLayout( grid );

    int row = -1;

    // Titles
    label = new QLabel(i18n("Use data from\nImport File") );
    grid->addWidget( label, ++row, 1 );

    label = new QLabel( i18n("Use data from\nDatabase") );
    grid->addWidget( label, row, 2 );

    label = new QLabel( i18n("Merge data") );
    grid->addWidget( label, row, 3 );

    ClashInfo clashes = this->clashes( settings );
    createRow( grid, row, QString::fromLatin1("*Label*"), i18n("Label"), clashes.label,false );
    createRow( grid, row, QString::fromLatin1("*Description*"), i18n("Description"), clashes.description, true);
    Q_FOREACH( const QString& category, clashes.categories.keys() )
        createRow( grid, row, category, category, clashes.categories[category], true );

    vlay->addStretch(1);
}

/**
 * Return the number of images in the import set which has the same MD5 sum as those from the DB.
 */
int ImportExport::MD5CheckPage::countOfMD5Matches( const ImportSettings& settings )
{
    int count = 0;
    DB::ImageInfoList list = settings.selectedImages();
    Q_FOREACH( DB::ImageInfoPtr info, list ) {
        if ( DB::ImageDB::instance()->md5Map()->contains(info->MD5Sum()) )
            ++count;
    }
    return count;
}

ImportExport::ClashInfo ImportExport::MD5CheckPage::clashes(const ImportSettings& settings)
{
    QStringList myCategories;
    Q_FOREACH( ImportMatcher* matcher, settings.importMatchers() ) {
        myCategories.append( matcher->_myCategory );
    }

    ClashInfo res( myCategories );
    DB::ImageInfoList list = settings.selectedImages();
    Q_FOREACH( DB::ImageInfoPtr info, list ) {
        if ( !DB::ImageDB::instance()->md5Map()->contains(info->MD5Sum()) )
            continue;

        const QString& name = DB::ImageDB::instance()->md5Map()->lookup(info->MD5Sum());
        DB::ImageInfoPtr other = DB::ImageDB::instance()->info( Settings::SettingsData::instance()->imageDirectory() + name ); // JKP - ouch md5 is without full, while image db is with. That should be cleaned up!
        if ( info->label() != other->label() )
            res.label = true;
        if ( info->description() != other->description() )
            res.description = true;

        Q_FOREACH( ImportMatcher* matcher, settings.importMatchers() ) {
            const QString XMLFileCategory = matcher->_otherCategory;
            const QString DBCategory = matcher->_myCategory;
            if ( mapCategoriesToDB( matcher, info->itemsOfCategory( XMLFileCategory ) ) != other->itemsOfCategory( DBCategory ) )
                res.categories[DBCategory] = true;
        }
    }
    return res;
}

bool ImportExport::ClashInfo::anyClashes()
{
    if ( label || description )
        return true;

    for( QMap<QString,bool>::ConstIterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        if (categoryIt.value() )
            return true;
    }

    return false;

}

void ImportExport::MD5CheckPage::createRow( QGridLayout* layout, int& row, const QString& name, const QString& title, bool anyClashes, bool allowMerge )
{
    QLabel* label = new QLabel( title );
    label->setEnabled( anyClashes );
    layout->addWidget( label, ++row, 0 );
    QPalette pal = label->palette();
    const QColor col = pal.color(QPalette::Background).lighter(105);
    pal.setColor( QPalette::Background, col );
    if ( row % 2 ) {
        label->setPalette( pal );
        label->setAutoFillBackground(true);
    }

    QButtonGroup* group = new QButtonGroup(this);
    m_groups[name]=group;

    for ( int i = 1; i<4;++i ) {
        if ( i == 3 && !allowMerge )
            continue;

        QRadioButton* rb = new QRadioButton;
        layout->addWidget( rb, row, i  );
        group->addButton( rb, i );
        rb->setEnabled( anyClashes );
        if ( row % 2 ) {
            QPalette pal = rb->palette();
            pal.setColor( QPalette::Button, col );
            rb->setPalette( pal );
            rb->setAutoFillBackground(true);
        }
        if (i == 1 )
            rb->setChecked(true);
    }
}

Utilities::StringSet ImportExport::MD5CheckPage::mapCategoriesToDB( ImportMatcher* matcher, const Utilities::StringSet& items )
{
    Utilities::StringSet res;

    QMap<QString,QString> mappings;
    Q_FOREACH( CategoryMatch* match, matcher->_matchers ) {
        if ( match->_checkbox->isChecked() )
            mappings[match->_combobox->currentText()] = match->_text;
    }

    Q_FOREACH( const QString& item, items ) {
        if ( mappings.contains( item ) )
            res.insert(mappings[item]);
    }
    return res;
}

QMap<QString, ImportExport::ImportSettings::ImportAction> ImportExport::MD5CheckPage::settings()
{
    QMap<QString, ImportSettings::ImportAction> res;
    for( QMap<QString,QButtonGroup*>::Iterator it = m_groups.begin(); it != m_groups.end(); ++it ) {
        res.insert( it.key(), static_cast<ImportSettings::ImportAction>(it.data()->checkedId()) );
    }
    return res;
}


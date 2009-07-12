#include "MD5CheckPage.h"
#include <QFrame>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <klocale.h>
#include <QLabel>
#include "DB/ImageDB.h"
#include "DB/MD5Map.h"

ImportExport::ClashInfo::ClashInfo( const QStringList& categories )
    : label(false), description(false), orientation(false), date(false)
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
    createRow( grid, row, QString::fromLatin1( "*Orientation*" ), i18n("Orientation"), clashes.orientation, false );
    createRow( grid, row, QString::fromLatin1( "*Date*" ), i18n("Date and Time"), clashes.date, false );
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
    Q_FOREACH( const CategoryMatchSetting& matcher, settings.categoryMatchSetting() ) {
        myCategories.append( matcher.DBCategoryName() );
    }

    ClashInfo res( myCategories );
    DB::ImageInfoList list = settings.selectedImages();
    Q_FOREACH( DB::ImageInfoPtr info, list ) {
        if ( !DB::ImageDB::instance()->md5Map()->contains(info->MD5Sum()) )
            continue;

        const QString& name = DB::ImageDB::instance()->md5Map()->lookup(info->MD5Sum());
        DB::ImageInfoPtr other = DB::ImageDB::instance()->info( name, DB::RelativeToImageRoot );
        if ( info->label() != other->label() )
            res.label = true;
        if ( info->description() != other->description() )
            res.description = true;

        if ( info->angle() != other->angle() )
            res.orientation = true;

        if (info->date() != other->date() )
            res.date = true;

        Q_FOREACH( const CategoryMatchSetting& matcher, settings.categoryMatchSetting() ) {
            const QString XMLFileCategory = matcher.XMLCategoryName();
            const QString DBCategory = matcher.DBCategoryName();
            if ( mapCategoriesToDB( matcher, info->itemsOfCategory( XMLFileCategory ) ) != other->itemsOfCategory( DBCategory ) )
                res.categories[DBCategory] = true;
        }
    }
    return res;
}

bool ImportExport::ClashInfo::anyClashes()
{
    if ( label || description || orientation || date)
        return true;

    for( QMap<QString,bool>::ConstIterator categoryIt = categories.constBegin(); categoryIt != categories.constEnd(); ++categoryIt ) {
        if (categoryIt.value() )
            return true;
    }

    return false;

}

void ImportExport::MD5CheckPage::createRow( QGridLayout* layout, int& row, const QString& name, const QString& title, bool anyClashes, bool allowMerge )
{
    if ( row % 3 == 0 ) {
        QFrame* line = new QFrame;
        line->setFrameShape( QFrame::HLine );
        layout->addWidget( line, ++row, 0, 1, 4 );
    }

    QLabel* label = new QLabel( title );
    label->setEnabled( anyClashes );
    layout->addWidget( label, ++row, 0 );

    QButtonGroup* group = new QButtonGroup(this);
    m_groups[name]=group;

    for ( int i = 1; i<4;++i ) {
        if ( i == 3 && !allowMerge )
            continue;

        QRadioButton* rb = new QRadioButton;
        layout->addWidget( rb, row, i  );
        group->addButton( rb, i );
        rb->setEnabled( anyClashes );
        if (i == 1 )
            rb->setChecked(true);
    }
}

Utilities::StringSet ImportExport::MD5CheckPage::mapCategoriesToDB( const CategoryMatchSetting& matcher, const Utilities::StringSet& items )
{
    Utilities::StringSet res;

    Q_FOREACH( const QString& item, items ) {
        if ( matcher.XMLtoDB().contains( item ) )
            res.insert(matcher.XMLtoDB()[item]);
    }
    return res;
}

QMap<QString, ImportExport::ImportSettings::ImportAction> ImportExport::MD5CheckPage::settings()
{
    QMap<QString, ImportSettings::ImportAction> res;
    for( QMap<QString,QButtonGroup*>::Iterator it = m_groups.begin(); it != m_groups.end(); ++it ) {
        res.insert( it.key(), static_cast<ImportSettings::ImportAction>(it.value()->checkedId()) );
    }
    return res;
}


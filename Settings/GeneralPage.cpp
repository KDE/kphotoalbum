#include "GeneralPage.h"
#include <DB/ImageDB.h>
#include <DB/Category.h>
#include <KComboBox>
#include <klocale.h>
#include <QSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <Q3VGroupBox>
#include <QVBoxLayout>
#include "DB/CategoryCollection.h"
#include "SettingsData.h"

Settings::GeneralPage::GeneralPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* lay1 = new QVBoxLayout( this );

    Q3VGroupBox* box = new Q3VGroupBox( i18n( "New Images" ), this );
    lay1->addWidget( box );

    // Thrust time stamps
    QWidget* container = new QWidget( box );
    QLabel* timeStampLabel = new QLabel( i18n("Trust image dates:"), container );
    _trustTimeStamps = new KComboBox( container );
    _trustTimeStamps->addItems( QStringList() << i18n("Always") << i18n("Ask") << i18n("Never") );
    QHBoxLayout* hlay = new QHBoxLayout( container );
    hlay->addWidget( timeStampLabel );
    hlay->addWidget( _trustTimeStamps );
    hlay->addStretch( 1 );

    // Do EXIF rotate
    _useEXIFRotate = new QCheckBox( i18n( "Use EXIF orientation information" ), box );

    _useEXIFComments = new QCheckBox( i18n( "Use EXIF description" ), box );

    // Search for images on startup
    _searchForImagesOnStart = new QCheckBox( i18n("Search for new images and videos on startup"), box );
    _skipRawIfOtherMatches = new QCheckBox( i18n("Do not read RAW files if a matching JPEG/TIFF file exists"), box );

    // Datebar size
    container = new QWidget( this );
    lay1->addWidget( container );
    hlay = new QHBoxLayout( container );
    QLabel* datebarSize = new QLabel( i18n("Size of histogram columns in date bar:"), container );
    hlay->addWidget( datebarSize );
    _barWidth = new QSpinBox;
    _barWidth->setRange( 1, 100 );
    _barWidth->setSingleStep( 1 );
    hlay->addWidget( _barWidth );
    QLabel* cross = new QLabel( QString::fromLatin1( " x " ), container );
    hlay->addWidget( cross );
    _barHeight = new QSpinBox;
    _barHeight->setRange( 15, 100 );
    hlay->addWidget( _barHeight );
    hlay->addStretch( 1 );

    // Show splash screen
    _showSplashScreen = new QCheckBox( i18n("Show splash screen"), this );
    lay1->addWidget( _showSplashScreen );

    // Album Category
    QLabel* albumCategoryLabel = new QLabel( i18n("Category for virtual albums:" ), this );
    _albumCategory = new QComboBox;
    QHBoxLayout* lay7 = new QHBoxLayout;
    lay1->addLayout( lay7 );

    lay7->addWidget( albumCategoryLabel );
    lay7->addWidget( _albumCategory );
    lay7->addStretch(1);

     QList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
     for( QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it ) {
        _albumCategory->addItem( (*it)->text() );
    }

    lay1->addStretch( 1 );


    // Whats This
    QString txt;

    txt = i18n( "<p>KPhotoAlbum will try to read the image date from EXIF information in the image. "
                "If that fails it will try to get the date from the file's time stamp.</p>"
                "<p>However, this information will be wrong if the image was scanned in (you want the date the image "
                "was taken, not the date of the scan).</p>"
                "<p>If you only scan images, in contrast to sometimes using "
                "a digital camera, you should reply <b>no</b>. If you never scan images, you should reply <b>yes</b>, "
                "otherwise reply <b>ask</b>. This will allow you to decide whether the images are from "
                "the scanner or the camera, from session to session.</p>" );
    timeStampLabel->setWhatsThis( txt );
    _trustTimeStamps->setWhatsThis( txt );

    txt = i18n( "<p>JPEG images may contain information about rotation. "
                "If you have a reason for not using this information to get a default rotation of "
                "your images, uncheck this check box.</p>"
                "<p>Note: Your digital camera may not write this information into the images at all.</p>" );
    _useEXIFRotate->setWhatsThis( txt );

    txt = i18n( "<p>JPEG images may contain a description. "
               "Check this checkbox to specify if you want to use this as a "
               "default description for your images.</p>" );
    _useEXIFComments->setWhatsThis( txt );

    txt = i18n( "<p>KPhotoAlbum is capable of searching for new images and videos when started, this does, "
                "however, take some time, so instead you may wish to manually tell KPhotoAlbum to search for new images "
                "using <b>Maintenance->Rescan for new images</b></p>");
    _searchForImagesOnStart->setWhatsThis( txt );

    txt = i18n( "<p>KPhotoAlbum is capable of reading certain kinds of RAW images.  "
		"Some cameras store both a RAW image and a matching JPEG or TIFF image.  "
		"This causes duplicate images to be stored in KPhotoAlbum, which may be undesirable.  "
		"If this option is checked, KPhotoAlbum will not read RAW files for which matching image files also exist.</p>");
    _skipRawIfOtherMatches->setWhatsThis( txt );

    txt = i18n("<p>KPhotoAlbum shares plugins with other imaging applications, some of which have the concept of albums. "
               "KPhotoAlbum does not have this concept; nevertheless, for certain plugins to function, KPhotoAlbum behaves "
               "to the plugin system as if it did.</p>"
               "<p>KPhotoAlbum does this by defining the current album to be the current view - that is, all the images the "
               "browser offers to display.</p>"
               "<p>In addition to the current album, KPhotoAlbum must also be able to give a list of all albums; "
               "the list of all albums is defined in the following way:"
               "<ul><li>When KPhotoAlbum's browser displays the content of a category, say all People, then each item in this category "
               "will look like an album to the plugin.</li>"
               "<li>Otherwise, the category you specify using this option will be used; e.g. if you specify People "
               "with this option, then KPhotoAlbum will act as if you had just chosen to display people and then invoke "
               "the plugin which needs to know about all albums.</li></ul></p>"
               "<p>Most users would probably want to specify Events here.</p>");
    albumCategoryLabel->setWhatsThis( txt );
    _albumCategory->setWhatsThis( txt );

    txt = i18n( "Show the KPhotoAlbum splash screen on start up" );
    _showSplashScreen->setWhatsThis( txt );
}

void Settings::GeneralPage::loadSettings( Settings::SettingsData* opt )
{
    _trustTimeStamps->setCurrentIndex( opt->tTimeStamps() );
    _useEXIFRotate->setChecked( opt->useEXIFRotate() );
    _useEXIFComments->setChecked( opt->useEXIFComments() );
    _searchForImagesOnStart->setChecked( opt->searchForImagesOnStart() );
    _skipRawIfOtherMatches->setChecked( opt->skipRawIfOtherMatches() );
    _barWidth->setValue( opt->histogramSize().width() );
    _barHeight->setValue( opt->histogramSize().height() );
    _showSplashScreen->setChecked( opt->showSplashScreen() );
    DB::CategoryPtr cat = DB::ImageDB::instance()->categoryCollection()->categoryForName( opt->albumCategory() );
    if ( !cat )
        cat = DB::ImageDB::instance()->categoryCollection()->categories()[0];

    _albumCategory->setEditText( cat->text() );

}

void Settings::GeneralPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setTTimeStamps( (TimeStampTrust) _trustTimeStamps->currentIndex() );
    opt->setUseEXIFRotate( _useEXIFRotate->isChecked() );
    opt->setUseEXIFComments( _useEXIFComments->isChecked() );
    opt->setSearchForImagesOnStart( _searchForImagesOnStart->isChecked() );
    opt->setSkipRawIfOtherMatches( _skipRawIfOtherMatches->isChecked() );
    opt->setShowSplashScreen( _showSplashScreen->isChecked() );
    QString name = DB::ImageDB::instance()->categoryCollection()->nameForText( _albumCategory->currentText() );
    if ( name.isNull() )
        name = DB::ImageDB::instance()->categoryCollection()->categoryNames()[0];
    opt->setHistogramSize( QSize( _barWidth->value(), _barHeight->value() ) );

    opt->setAlbumCategory( name );
}

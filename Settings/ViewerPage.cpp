#include "ViewerPage.h"
#include "SettingsData.h"
#include <klocale.h>
#include <KComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include "ViewerSizeConfig.h"
#include <QVBoxLayout>

Settings::ViewerPage::ViewerPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* lay1 = new QVBoxLayout( this );

    _slideShowSetup = new ViewerSizeConfig( i18n( "Running Slide Show From Thumbnail View" ), this, "_slideShowSetup" );
    lay1->addWidget( _slideShowSetup );

    _viewImageSetup = new ViewerSizeConfig( i18n( "Viewing Images and Videos From Thumbnail View" ), this, "_viewImageSetup" );
    lay1->addWidget( _viewImageSetup );

    QGridLayout* glay = new QGridLayout;
    lay1->addLayout( glay );

    QLabel* label = new QLabel( i18n("Slideshow interval:" ), this );
    glay->addWidget( label, 0, 0 );

    _slideShowInterval = new QSpinBox;
    _slideShowInterval->setRange( 1, INT_MAX );
    glay->addWidget( _slideShowInterval, 0, 1 );
    _slideShowInterval->setSuffix( i18n( " sec" ) );
    label->setBuddy( _slideShowInterval );

    label = new QLabel( i18n("Image cache:"), this );
    glay->addWidget( label, 1, 0 );

    _cacheSize = new QSpinBox;
    _cacheSize->setRange( 0, 4096 );
    _cacheSize->setSingleStep( 10 );
    _cacheSize->setSuffix( i18n(" Mbytes") );
    glay->addWidget( _cacheSize, 1, 1 );
    label->setBuddy( _cacheSize );

    QString txt;

    QLabel* standardSizeLabel = new QLabel( i18n("Standard size in viewer:"), this );
    _viewerStandardSize = new KComboBox( this );
    _viewerStandardSize->addItems( QStringList() << i18n("Full Viewer Size") << i18n("Natural Image Size") << i18n("Natural Image Size If Possible") );
    glay->addWidget( standardSizeLabel, 2, 0);
    glay->addWidget( _viewerStandardSize, 2, 1 );
    standardSizeLabel->setBuddy( _viewerStandardSize );

    txt = i18n("<p>Set the standard size for images to be displayed in the viewer.</p> "
	       "<p><b>Full Viewer Size</b> indicates that the image will be stretched or shrunk to fill the viewer window.</p> "
	       "<p><b>Natural Image Size</b> indicates that the image will be displayed pixel for pixel.</p> "
	       "<p><b>Natural Image Size If Possible</b> indicates that the image will be displayed pixel for pixel if it would fit the window, "
	       "otherwise it will be shrunk to fit the viewer.</p>");
    _viewerStandardSize->setWhatsThis( txt);

    QLabel* scalingLabel = new QLabel( i18n("Scaling Algorithm"), this );
    _smoothScale = new QComboBox( this );
    _smoothScale->addItems( QStringList() << i18n("Fastest" ) << i18n("Best")  );
    scalingLabel->setBuddy( _smoothScale );

    glay->addWidget( scalingLabel, 3, 0 );
    glay->addWidget( _smoothScale, 3, 1 );
    txt = i18n("<p>When displaying images, KPhotoAlbum normally performs smooth scaling of the image. "
		       "If this option is not set, KPhotoAlbum will use a faster but less smooth scaling method.</p>");
    scalingLabel->setWhatsThis( txt );
    _smoothScale->setWhatsThis( txt );
}

void Settings::ViewerPage::loadSettings( Settings::SettingsData* opt )
{
    _viewImageSetup->setSize( opt->viewerSize() );
    _slideShowSetup->setSize( opt->slideShowSize() );
    _slideShowInterval->setValue( opt->slideShowInterval() );
    _cacheSize->setValue( opt->viewerCacheSize() );
    _smoothScale->setCurrentIndex( opt->smoothScale() );
    _viewerStandardSize->setCurrentIndex( opt->viewerStandardSize() );
}

void Settings::ViewerPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setLaunchViewerFullScreen( _viewImageSetup->launchFullScreen() );
    opt->setViewerSize( _viewImageSetup->size() );
    opt->setSlideShowInterval( _slideShowInterval->value() );
    opt->setViewerCacheSize( _cacheSize->value() );
    opt->setSmoothScale( _smoothScale->currentIndex() );
    opt->setViewerStandardSize((StandardViewSize) _viewerStandardSize->currentIndex());
    opt->setSlideShowSize( _slideShowSetup->size() );
    opt->setLaunchSlideShowFullScreen( _slideShowSetup->launchFullScreen() );
}

void Settings::ViewerPage::reset(Settings::SettingsData* opt )
{
    _viewImageSetup->setLaunchFullScreen( opt->launchViewerFullScreen() );
    _slideShowSetup->setLaunchFullScreen( opt->launchSlideShowFullScreen() );
}

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
#include "ViewerPage.h"
#include "SettingsData.h"
#include <KLocalizedString>
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

    m_slideShowSetup = new ViewerSizeConfig( i18n( "Running Slide Show From Thumbnail View" ), this );
    lay1->addWidget( m_slideShowSetup );

    m_viewImageSetup = new ViewerSizeConfig( i18n( "Viewing Images and Videos From Thumbnail View" ), this );
    lay1->addWidget( m_viewImageSetup );

    QGridLayout* glay = new QGridLayout;
    lay1->addLayout( glay );

    QLabel* label = new QLabel( i18n("Slideshow interval:" ), this );
    glay->addWidget( label, 0, 0 );

    m_slideShowInterval = new QSpinBox;
    m_slideShowInterval->setRange( 1, INT_MAX );
    glay->addWidget( m_slideShowInterval, 0, 1 );
    m_slideShowInterval->setSuffix( i18n( " sec" ) );
    label->setBuddy( m_slideShowInterval );

    label = new QLabel( i18n("Image cache:"), this );
    glay->addWidget( label, 1, 0 );

    m_cacheSize = new QSpinBox;
    m_cacheSize->setRange( 0, 16384 );
    m_cacheSize->setSingleStep( 10 );
    m_cacheSize->setSuffix( i18n(" Mbytes") );
    glay->addWidget( m_cacheSize, 1, 1 );
    label->setBuddy( m_cacheSize );

    QString txt;

    QLabel* standardSizeLabel = new QLabel( i18n("Standard size in viewer:"), this );
    m_viewerStandardSize = new KComboBox( this );
    m_viewerStandardSize->addItems( QStringList() << i18n("Full Viewer Size") << i18n("Natural Image Size") << i18n("Natural Image Size If Possible") );
    glay->addWidget( standardSizeLabel, 2, 0);
    glay->addWidget( m_viewerStandardSize, 2, 1 );
    standardSizeLabel->setBuddy( m_viewerStandardSize );

    txt = i18n("<p>Set the standard size for images to be displayed in the viewer.</p> "
           "<p><b>Full Viewer Size</b> indicates that the image will be stretched or shrunk to fill the viewer window.</p> "
           "<p><b>Natural Image Size</b> indicates that the image will be displayed pixel for pixel.</p> "
           "<p><b>Natural Image Size If Possible</b> indicates that the image will be displayed pixel for pixel if it would fit the window, "
           "otherwise it will be shrunk to fit the viewer.</p>");
    m_viewerStandardSize->setWhatsThis( txt);

    QLabel* scalingLabel = new QLabel( i18n("Scaling algorithm:"), this );
    m_smoothScale = new KComboBox( this );
    m_smoothScale->addItems( QStringList() << i18n("Fastest" ) << i18n("Best")  );
    scalingLabel->setBuddy( m_smoothScale );

    glay->addWidget( scalingLabel, 3, 0 );
    glay->addWidget( m_smoothScale, 3, 1 );
    txt = i18n("<p>When displaying images, KPhotoAlbum normally performs smooth scaling of the image. "
               "If this option is not set, KPhotoAlbum will use a faster but less smooth scaling method.</p>");
    scalingLabel->setWhatsThis( txt );
    m_smoothScale->setWhatsThis( txt );
}

void Settings::ViewerPage::loadSettings( Settings::SettingsData* opt )
{
    m_viewImageSetup->setLaunchFullScreen( opt->launchViewerFullScreen() );
    m_viewImageSetup->setSize( opt->viewerSize() );
    m_slideShowSetup->setLaunchFullScreen( opt->launchSlideShowFullScreen() );
    m_slideShowSetup->setSize( opt->slideShowSize() );
    m_slideShowInterval->setValue( opt->slideShowInterval() );
    m_cacheSize->setValue( opt->viewerCacheSize() );
    m_smoothScale->setCurrentIndex( opt->smoothScale() );
    m_viewerStandardSize->setCurrentIndex( opt->viewerStandardSize() );
}

void Settings::ViewerPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setLaunchViewerFullScreen( m_viewImageSetup->launchFullScreen() );
    opt->setViewerSize( m_viewImageSetup->size() );
    opt->setSlideShowInterval( m_slideShowInterval->value() );
    opt->setViewerCacheSize( m_cacheSize->value() );
    opt->setSmoothScale( m_smoothScale->currentIndex() );
    opt->setViewerStandardSize((StandardViewSize) m_viewerStandardSize->currentIndex());
    opt->setSlideShowSize( m_slideShowSetup->size() );
    opt->setLaunchSlideShowFullScreen( m_slideShowSetup->launchFullScreen() );
}
// vi:expandtab:tabstop=4 shiftwidth=4:

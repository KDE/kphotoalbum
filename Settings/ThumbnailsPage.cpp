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
#include "ThumbnailsPage.h"
#include <KColorButton>
#include "SettingsData.h"
#include <KLocalizedString>
#include <QCheckBox>
#include <KComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>

Settings::ThumbnailsPage::ThumbnailsPage( QWidget* parent )
    : QWidget( parent )
{
    QGridLayout* lay = new QGridLayout( this );
    lay->setSpacing( 6 );
    int row = 0;

    // Preview size
    QLabel* previewSizeLabel = new QLabel( i18n("Tooltip preview image size:" ) );
    m_previewSize = new QSpinBox;
    m_previewSize->setRange( 0, 2000 );
    m_previewSize->setSingleStep( 10 );
    m_previewSize->setSpecialValueText( i18n("No Image Preview") );
    lay->addWidget( previewSizeLabel, row, 0 );
    lay->addWidget( m_previewSize, row, 1 );

    // Thumbnail size
    ++row;
    QLabel* thumbnailSizeLabel = new QLabel( i18n("Thumbnail image size:" ) );
    m_thumbnailSize = new QSpinBox;
    // range set from settings on load
    m_thumbnailSize->setSingleStep( 16 );
    lay->addWidget( thumbnailSizeLabel, row, 0 );
    lay->addWidget( m_thumbnailSize, row, 1 );

    // incremental Thumbnail building
    ++row;
    m_incrementalThumbnails = new QCheckBox( i18n("Build thumbnails on demand" ) );
    lay->addWidget( m_incrementalThumbnails, row, 0, 1, 2);

    // Thumbnail aspect ratio
    ++row;
    QLabel* thumbnailAspectRatioLabel = new QLabel( i18n("Thumbnail table cells aspect ratio:") );
    m_thumbnailAspectRatio = new KComboBox( this );
    m_thumbnailAspectRatio->addItems( QStringList() << i18n("1:1") << i18n("4:3")
        << i18n("3:2") << i18n("16:9") << i18n("3:4") << i18n("2:3") << i18n("9:16"));
    lay->addWidget( thumbnailAspectRatioLabel, row, 0 );
    lay->addWidget( m_thumbnailAspectRatio, row, 1 );

    // Space around cells
    ++row;
    QLabel* thumbnailSpaceLabel = new QLabel( i18n("Space around cells:") );
    m_thumbnailSpace = new QSpinBox;
    m_thumbnailSpace->setRange( 0, 20 );
    lay->addWidget( thumbnailSpaceLabel, row, 0 );
    lay->addWidget( m_thumbnailSpace, row, 1 );

    // Background color
    ++row;
    QLabel* backgroundColorLabel = new QLabel( i18n("Background color:") );
    m_backgroundColor = new KColorButton;
    lay->addWidget( backgroundColorLabel, row, 0 );
    lay->addWidget( m_backgroundColor, row, 1 );

    // Display grid lines in the thumbnail view
    ++row;
    m_thumbnailDisplayGrid = new QCheckBox( i18n("Display grid around thumbnails" ) );
    lay->addWidget( m_thumbnailDisplayGrid, row, 0, 1, 2 );

    // Display Labels
    ++row;
    m_displayLabels = new QCheckBox( i18n("Display labels in thumbnail view" ) );
    lay->addWidget( m_displayLabels, row, 0, 1, 2 );

    // Display Categories
    ++row;
    m_displayCategories = new QCheckBox( i18n("Display categories in thumbnail view" ) );
    lay->addWidget( m_displayCategories, row, 0, 1, 2 );

    // Auto Show Thumbnail view
    ++row;
    QLabel* autoShowLabel = new QLabel( i18n("Threshold for automatic thumbnail view: "), this );
    m_autoShowThumbnailView = new QSpinBox;
    m_autoShowThumbnailView->setRange( 0, 10000 );
    m_autoShowThumbnailView->setSingleStep( 10 );
    m_autoShowThumbnailView->setSpecialValueText( i18nc("Describing: 'ThumbnailView will not be automatically shown'","Disabled") );
    lay->addWidget( autoShowLabel, row, 0 );
    lay->addWidget( m_autoShowThumbnailView, row, 1 );


    lay->setColumnStretch( 1, 1 );
    lay->setRowStretch( ++row, 1 );

    // Whats This
    QString txt;

    txt = i18n( "<p>If you select <b>Settings -&gt; Show Tooltips</b> in the thumbnail view, then you will see a small tool tip window "
                "displaying information about the thumbnails. This window includes a small preview image. "
                "This option configures the image size.</p>" );
    previewSizeLabel->setWhatsThis( txt );
    m_previewSize->setWhatsThis( txt );


    txt = i18n( "<p>Thumbnail image size. Changing the thumbnail size here triggers a rebuild of the thumbnail database.</p>" );
    thumbnailSizeLabel->setWhatsThis( txt );
    m_thumbnailSize->setWhatsThis( txt );

    txt = i18n( "<p>If this is set, thumbnails are built on demand. As you browse your image database, "
                "only those thumbnails that are needed are actually built. "
                "This means that when you change the thumbnail size, KPhotoAlbum will remain responsive "
                "even if you have lots of images.</p>"
                "<p>If this is not set, KPhotoAlbum will always build the thumbnails for all images as soon as possible. "
                "This means that when new images are found, KPhotoAlbum will immediately build thumbnails "
                "for them and you won't have a delay later while browsing.</p>");
    m_incrementalThumbnails->setWhatsThis( txt );

    txt = i18n("<p>Choose what aspect ratio the cells holding thumbnails should have.</p>");
    m_thumbnailAspectRatio->setWhatsThis( txt );

    txt = i18n("<p>How thick the cell padding should be.</p>");
    thumbnailSpaceLabel->setWhatsThis( txt );

    txt = i18n("<p>Background color to use in the thumbnail viewer</p>");
    backgroundColorLabel->setWhatsThis( txt );
    m_backgroundColor->setWhatsThis( txt );

    txt = i18n("<p>If you want to see grid around your thumbnail images, "
               "select this option.</p>");
    m_thumbnailDisplayGrid->setWhatsThis( txt );

    txt = i18n("<p>Checking this option will show the base name for the file under "
               "thumbnails in the thumbnail view.</p>");
    m_displayLabels->setWhatsThis( txt );

    txt = i18n("<p>Checking this option will show the Categories for the file under "
        "thumbnails in the thumbnail view</p>");
    m_displayCategories->setWhatsThis( txt );

    txt = i18n("<p>When you are browsing, and the count gets below the value specified here, "
               "the thumbnails will be shown automatically. The alternative is to continue showing the "
               "browser until you press <i>Show Images</i></p>");
    m_autoShowThumbnailView->setWhatsThis( txt );
    autoShowLabel->setWhatsThis( txt );
}

void Settings::ThumbnailsPage::loadSettings( Settings::SettingsData* opt )
{
    m_previewSize->setValue( opt->previewSize() );
    m_thumbnailSize->setMinimum( opt->minimumThumbnailSize() );
    m_thumbnailSize->setMaximum( opt->maximumThumbnailSize() );
    m_thumbnailSize->setValue( opt->thumbnailSize() );
    m_backgroundColor->setColor( QColor( opt->backgroundColor() ) );
    m_thumbnailDisplayGrid->setChecked( opt->thumbnailDisplayGrid() );
    m_thumbnailAspectRatio->setCurrentIndex( opt->thumbnailAspectRatio() );
    m_thumbnailSpace->setValue( opt->thumbnailSpace() );
    m_displayLabels->setChecked( opt->displayLabels() );
    m_displayCategories->setChecked( opt->displayCategories() );
    m_autoShowThumbnailView->setValue( opt->autoShowThumbnailView() );
    m_incrementalThumbnails->setChecked( opt->incrementalThumbnails() );
}

void Settings::ThumbnailsPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setPreviewSize( m_previewSize->value() );
    opt->setThumbnailSize( m_thumbnailSize->value() );
    // ensure that the user actually sees the thumbnail size change:
    opt->setActualThumbnailSize( m_thumbnailSize->value() );
    opt->setThumbnailAspectRatio( (ThumbnailAspectRatio) m_thumbnailAspectRatio->currentIndex() );
    opt->setBackgroundColor( m_backgroundColor->color().name() );
    opt->setThumbnailDisplayGrid( m_thumbnailDisplayGrid->isChecked() );
    opt->setThumbnailSpace( m_thumbnailSpace->value() );
    opt->setDisplayLabels( m_displayLabels->isChecked() );
    opt->setDisplayCategories( m_displayCategories->isChecked() );
    opt->setAutoShowThumbnailView( m_autoShowThumbnailView->value() );
    opt->setIncrementalThumbnails( m_incrementalThumbnails->isChecked() );
}


// vi:expandtab:tabstop=4 shiftwidth=4:

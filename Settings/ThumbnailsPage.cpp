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
#include <klocale.h>
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
    _previewSize = new QSpinBox;
    _previewSize->setRange( 0, 2000 );
    _previewSize->setSingleStep( 10 );
    _previewSize->setSpecialValueText( i18n("No Image Preview") );
    lay->addWidget( previewSizeLabel, row, 0 );
    lay->addWidget( _previewSize, row, 1 );

    // Thumbnail size
    ++row;
    QLabel* thumbnailSizeLabel = new QLabel( i18n("Thumbnail image size:" ) );
    _thumbnailSize = new QSpinBox;
    _thumbnailSize->setRange( 32, 4096 );
    _thumbnailSize->setSingleStep( 16 );
    lay->addWidget( thumbnailSizeLabel, row, 0 );
    lay->addWidget( _thumbnailSize, row, 1 );

    // Thumbnail aspect ratio
    ++row;
    QLabel* thumbnailAspectRatioLabel = new QLabel( i18n("Thumbnail table cells aspect ratio") );
    _thumbnailAspectRatio = new KComboBox( this );
    _thumbnailAspectRatio->addItems( QStringList() << i18n("1:1") << i18n("4:3")
        << i18n("3:2") << i18n("16:9") << i18n("3:4") << i18n("2:3") << i18n("9:16"));
    lay->addWidget( thumbnailAspectRatioLabel, row, 0 );
    lay->addWidget( _thumbnailAspectRatio, row, 1 );

    // Space around cells
    ++row;
    QLabel* thumbnailSpaceLabel = new QLabel( i18n("Space around cells") );
    _thumbnailSpace = new QSpinBox;
    _thumbnailSpace->setRange( 0, 20 );
    lay->addWidget( thumbnailSpaceLabel, row, 0 );
    lay->addWidget( _thumbnailSpace, row, 1 );

    // Background color
    ++row;
    QLabel* backgroundColorLabel = new QLabel( i18n("Background Color") );
    _backgroundColor = new KColorButton;
    lay->addWidget( backgroundColorLabel, row, 0 );
    lay->addWidget( _backgroundColor, row, 1 );

    // Display grid lines in the thumbnail view
    ++row;
    _thumbnailDisplayGrid = new QCheckBox( i18n("Display grid around thumbnails" ) );
    lay->addWidget( _thumbnailDisplayGrid, row, 0, 1, 2 );

    // Display Labels
    ++row;
    _displayLabels = new QCheckBox( i18n("Display labels in thumbnail view" ) );
    lay->addWidget( _displayLabels, row, 0, 1, 2 );

    // Display Categories
    ++row;
    _displayCategories = new QCheckBox( i18n("Display categories in thumbnail view" ) );
    lay->addWidget( _displayCategories, row, 0, 1, 2 );

    // Auto Show Thumbnail view
    ++row;
    QLabel* autoShowLabel = new QLabel( i18n("Threshold for automatic thumbnail view: "), this );
    _autoShowThumbnailView = new QSpinBox;
    _autoShowThumbnailView->setRange( 0, 10000 );
    _autoShowThumbnailView->setSingleStep( 10 );
    _autoShowThumbnailView->setSpecialValueText( i18nc("Describing: 'ThumbnailView will not be automatically shown'","Disabled") );
    lay->addWidget( autoShowLabel, row, 0 );
    lay->addWidget( _autoShowThumbnailView, row, 1 );


    lay->setColumnStretch( 1, 1 );
    lay->setRowStretch( ++row, 1 );

    // Whats This
    QString txt;

    txt = i18n( "<p>If you select <b>Settings -&gt; Show Tooltips</b> in the thumbnail view, then you will see a small tool tip window "
                "displaying information about the thumbnails. This window includes a small preview image. "
                "This option configures the image size.</p>" );
    previewSizeLabel->setWhatsThis( txt );
    _previewSize->setWhatsThis( txt );


    txt = i18n( "<p>Thumbnail image size. You may also set the size simply by dragging the thumbnail view using the middle mouse button.</p>" );
    thumbnailSizeLabel->setWhatsThis( txt );
    _thumbnailSize->setWhatsThis( txt );

    txt = i18n("<p>Choose what aspect ratio the cells holding thumbnails should have.</p>");
    _thumbnailAspectRatio->setWhatsThis( txt );

    txt = i18n("<p>How thick the cell padding should be.</p>");
    thumbnailSpaceLabel->setWhatsThis( txt );

    txt = i18n("<p>Background color to use in the thumbnail viewer</p>");
    backgroundColorLabel->setWhatsThis( txt );
    _backgroundColor->setWhatsThis( txt );

    txt = i18n("<p>If you want to see grid around your thumbnail images, "
               "select this option.</p>");
    _thumbnailDisplayGrid->setWhatsThis( txt );

    txt = i18n("<p>Checking this option will show the base name for the file under "
               "thumbnails in the thumbnail view.</p>");
    _displayLabels->setWhatsThis( txt );

    txt = i18n("<p>Checking this option will show the Categories for the file under "
        "thumbnails in the thumbnail view</p>");
    _displayCategories->setWhatsThis( txt );

    txt = i18n("<p>When you are browsing, and the count gets below the value specified here, "
               "the thumbnails will be shown automatically. The alternative is to continue showing the "
               "browser until you press <i>Show Images</i></p>");
    _autoShowThumbnailView->setWhatsThis( txt );
    autoShowLabel->setWhatsThis( txt );
}

void Settings::ThumbnailsPage::loadSettings( Settings::SettingsData* opt )
{
    _previewSize->setValue( opt->previewSize() );
    _thumbnailSize->setValue( opt->thumbSize() );
    _backgroundColor->setColor( QColor( opt->backgroundColor() ) );
    _thumbnailDisplayGrid->setChecked( opt->thumbnailDisplayGrid() );
    _thumbnailAspectRatio->setCurrentIndex( opt->thumbnailAspectRatio() );
    _thumbnailSpace->setValue( opt->thumbnailSpace() );
    _displayLabels->setChecked( opt->displayLabels() );
    _displayCategories->setChecked( opt->displayCategories() );
    _autoShowThumbnailView->setValue( opt->autoShowThumbnailView() );
}

void Settings::ThumbnailsPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setPreviewSize( _previewSize->value() );
    opt->setThumbSize( _thumbnailSize->value() );
    opt->setThumbnailAspectRatio( (ThumbnailAspectRatio) _thumbnailAspectRatio->currentIndex() );
    opt->setBackgroundColor( _backgroundColor->color().name() );
    opt->setThumbnailDisplayGrid( _thumbnailDisplayGrid->isChecked() );
    opt->setThumbnailSpace( _thumbnailSpace->value() );
    opt->setDisplayLabels( _displayLabels->isChecked() );
    opt->setDisplayCategories( _displayCategories->isChecked() );
    opt->setAutoShowThumbnailView( _autoShowThumbnailView->value() );
}

bool Settings::ThumbnailsPage::thumbnailSizeChanged( Settings::SettingsData* opt ) const
{
    return _thumbnailSize->value() != opt->thumbSize();
}


// vi:expandtab:tabstop=4 shiftwidth=4:

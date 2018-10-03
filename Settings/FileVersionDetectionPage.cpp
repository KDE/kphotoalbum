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
#include "FileVersionDetectionPage.h"
#include "SettingsData.h"
#include <KLocalizedString>
#include <QLabel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QGroupBox>

Settings::FileVersionDetectionPage::FileVersionDetectionPage( QWidget* parent )
    : QWidget( parent )
{
    QVBoxLayout* topLayout = new QVBoxLayout( this );
    QString txt;

    // General file searching
    {
        QGroupBox* generalBox = new QGroupBox( i18n("New File Searches"), this );
        topLayout->addWidget( generalBox );
        QVBoxLayout* layout = new QVBoxLayout(generalBox);

        // Search for images on startup
        m_searchForImagesOnStart = new QCheckBox( i18n("Search for new images and videos on startup"), generalBox );
        layout->addWidget(m_searchForImagesOnStart);

        m_ignoreFileExtension = new QCheckBox( i18n("Ignore file extensions when searching for new images and videos"), generalBox);
        layout->addWidget(m_ignoreFileExtension);

        m_skipSymlinks = new QCheckBox( i18n("Skip symbolic links when searching for new images"), generalBox );
        layout->addWidget(m_skipSymlinks);

        m_skipRawIfOtherMatches = new QCheckBox( i18n("Do not read RAW files if a matching JPEG/TIFF file exists"), generalBox );
        layout->addWidget(m_skipRawIfOtherMatches);

        // Exclude directories from search
        QLabel* excludeDirectoriesLabel = new QLabel( i18n("Directories to exclude from new file search:" ), generalBox );
        layout->addWidget(excludeDirectoriesLabel);

        m_excludeDirectories = new QLineEdit( generalBox );
        layout->addWidget(m_excludeDirectories);
        excludeDirectoriesLabel->setBuddy( m_excludeDirectories );

        txt = i18n( "<p>KPhotoAlbum is capable of searching for new images and videos when started, this does, "
                    "however, take some time, so instead you may wish to manually tell KPhotoAlbum to search for new images "
                    "using <b>Maintenance->Rescan for new images</b></p>");
        m_searchForImagesOnStart->setWhatsThis( txt );

        txt = i18n( "<p>KPhotoAlbum will normally search new images and videos by their file extension. "
                    "If this option is set, <em>all</em> files neither in the database nor in the block list "
                    "will be checked by their Mime type, regardless of their extension. This will take "
                    "significantly longer than finding files by extension!</p>");
        m_ignoreFileExtension->setWhatsThis( txt );

        txt = i18n( "<p>KPhotoAlbum attempts to read all image files whether actual files or symbolic links. If you "
                    "wish to ignore symbolic links, check this option. This is useful if for some reason you have e.g. "
                    "both the original files and symbolic links to these files within your image directory.</p>");
        m_skipSymlinks->setWhatsThis( txt );

        txt = i18n( "<p>KPhotoAlbum is capable of reading certain kinds of RAW images. "
                    "Some cameras store both a RAW image and a matching JPEG or TIFF image. "
                    "This causes duplicate images to be stored in KPhotoAlbum, which may be undesirable. "
                    "If this option is checked, KPhotoAlbum will not read RAW files for which matching image files also exist.</p>");
        m_skipRawIfOtherMatches->setWhatsThis( txt );

        txt = i18n( "<p>Directories defined here (separated by comma <tt>,</tt>) are "
                    "skipped when searching for new photos. Thumbnail directories of different "
                    "tools should be configured here. E.g. <tt>xml,ThumbNails,.thumbs,.thumbnails</tt>.</p>" );
        excludeDirectoriesLabel->setWhatsThis( txt );

    }

    // Original/Modified File Support
    {
        QGroupBox* modifiedBox = new QGroupBox( i18n("File Version Detection Settings"), this );
        topLayout->addWidget( modifiedBox );
        QVBoxLayout* layout = new QVBoxLayout(modifiedBox);

        m_detectModifiedFiles = new QCheckBox(i18n("Try to detect multiple versions of files"), modifiedBox);
        layout->addWidget(m_detectModifiedFiles);

        QLabel* modifiedFileComponentLabel = new QLabel( i18n("File versions search regexp:" ), modifiedBox );
        layout->addWidget(modifiedFileComponentLabel);

        m_modifiedFileComponent = new QLineEdit(modifiedBox);
        layout->addWidget(m_modifiedFileComponent);

        QLabel* originalFileComponentLabel = new QLabel( i18n("Original file replacement text:" ), modifiedBox );
        layout->addWidget(originalFileComponentLabel);

        m_originalFileComponent = new QLineEdit(modifiedBox);
        layout->addWidget(m_originalFileComponent);

        m_moveOriginalContents = new QCheckBox(i18n("Move meta-data (i.e. delete tags from the original)"), modifiedBox);
        layout->addWidget(m_moveOriginalContents);

        m_autoStackNewFiles = new QCheckBox(i18n("Automatically stack new versions of images"), modifiedBox);
        layout->addWidget(m_autoStackNewFiles);

        txt = i18n( "<p>When KPhotoAlbum searches for new files and finds a file that matches the "
                    "<i>modified file search regexp</i> it is assumed that an original version of "
                    "the image may exist. The regexp pattern will be replaced with the <i>original "
                    "file replacement text</i> and if that file exists, all associated metadata (category "
                    "information, ratings, etc) will be copied or moved from the original file to the new one.</p>");
        m_detectModifiedFiles->setWhatsThis( txt );

        txt = i18n( "<p>A perl regular expression that should match a modified file. "
                    "<ul><li>A dot matches a single character (<tt>\\.</tt> matches a dot)</li> "
                    "  <li>You can use the quantifiers <tt>*</tt>,<tt>+</tt>,<tt>?</tt>, or you can "
                    "    match multiple occurrences of an expression by using curly brackets (e.g. "
                    "<tt>e{0,1}</tt> matches 0 or 1 occurrences of the character \"e\").</li> "
                    "  <li>You can group parts of the expression using parenthesis.</li> "
                    "</ul>Example: <tt>-modified\\.(jpg|tiff)</tt> </p>");
        modifiedFileComponentLabel->setWhatsThis( txt );
        m_modifiedFileComponent->setWhatsThis( txt );

        txt = i18n( "<p>A string that is used to replace the match from the <i>File versions search regexp</i>. "
                    "This can be a semicolon (<tt>;</tt>) separated list. Each string is used to replace the match "
                    "in the new file's name until an original file is found or we run out of options.</p>");
        originalFileComponentLabel->setWhatsThis( txt );
        m_originalFileComponent->setWhatsThis( txt );

        txt = i18n( "<p>The tagging is moved from the original file to the new file. This way "
                    "only the latest version of an image is tagged.</p>" );
        m_moveOriginalContents->setWhatsThis( txt );

        txt = i18n( "<p>If this option is set, new versions of an image are automatically stacked "
                    "and placed to the top of the stack. This way the new image is shown when the "
                    "stack is in collapsed state - the default state in KPhotoAlbum.</p>" );
        m_autoStackNewFiles->setWhatsThis( txt );
    }

    // Copy File Support
    {
        QGroupBox* copyBox = new QGroupBox( i18nc("Configure the feature to make a copy of a file first and then open the copied file with an external application", "Copy File and Open with an External Application"), this );
        topLayout->addWidget( copyBox );
        QVBoxLayout* layout = new QVBoxLayout(copyBox);

        QLabel* copyFileComponentLabel = new QLabel( i18n("Copy file search regexp:" ), copyBox );
        layout->addWidget(copyFileComponentLabel);

        m_copyFileComponent = new QLineEdit(copyBox);
        layout->addWidget(m_copyFileComponent);

        QLabel* copyFileReplacementComponentLabel = new QLabel( i18n("Copy file replacement text:" ), copyBox );
        layout->addWidget(copyFileReplacementComponentLabel);

        m_copyFileReplacementComponent = new QLineEdit(copyBox);
        layout->addWidget(m_copyFileReplacementComponent);

        txt = i18n("<p>KPhotoAlbum can make a copy of an image before opening it with an external application. This configuration defines how the new file is named.</p>"
                   "<p>The regular expression defines the part of the original file name that is replaced with the <i>replacement text</i>. "
                   "E.g. regexp <i>\"\\.(jpg|png)\"</i> and replacement text <i>\"-mod.\\1\"</i> would copy test.jpg to test-mod.jpg and open the new file in selected application.</p>");
        copyFileComponentLabel->setWhatsThis( txt );
        m_copyFileComponent->setWhatsThis( txt );
        copyFileReplacementComponentLabel->setWhatsThis( txt );
        m_copyFileReplacementComponent->setWhatsThis( txt );
    }

}

Settings::FileVersionDetectionPage::~FileVersionDetectionPage()
{
    delete m_searchForImagesOnStart;
    delete m_ignoreFileExtension;
    delete m_skipSymlinks;
    delete m_skipRawIfOtherMatches;
    delete m_excludeDirectories;
    delete m_detectModifiedFiles;
    delete m_modifiedFileComponent;
    delete m_originalFileComponent;
    delete m_moveOriginalContents;
    delete m_autoStackNewFiles;
    delete m_copyFileComponent;
    delete m_copyFileReplacementComponent;
}

void Settings::FileVersionDetectionPage::loadSettings( Settings::SettingsData* opt )
{
    m_searchForImagesOnStart->setChecked( opt->searchForImagesOnStart() );
    m_ignoreFileExtension->setChecked( opt->ignoreFileExtension() );
    m_skipSymlinks->setChecked( opt->skipSymlinks() );
    m_skipRawIfOtherMatches->setChecked( opt->skipRawIfOtherMatches() );
    m_excludeDirectories->setText( opt->excludeDirectories() );
    m_detectModifiedFiles->setChecked( opt->detectModifiedFiles() );
    m_modifiedFileComponent->setText( opt->modifiedFileComponent() );
    m_originalFileComponent->setText( opt->originalFileComponent() );
    m_moveOriginalContents->setChecked( opt->moveOriginalContents() );
    m_autoStackNewFiles->setChecked( opt->autoStackNewFiles() );
    m_copyFileComponent->setText( opt->copyFileComponent() );
    m_copyFileReplacementComponent->setText( opt->copyFileReplacementComponent() );
}

void Settings::FileVersionDetectionPage::saveSettings( Settings::SettingsData* opt )
{
    opt->setSearchForImagesOnStart( m_searchForImagesOnStart->isChecked() );
    opt->setIgnoreFileExtension( m_ignoreFileExtension->isChecked() );
    opt->setSkipSymlinks( m_skipSymlinks->isChecked() );
    opt->setSkipRawIfOtherMatches( m_skipRawIfOtherMatches->isChecked() );
    opt->setExcludeDirectories( m_excludeDirectories->text() );
    opt->setDetectModifiedFiles( m_detectModifiedFiles->isChecked() );
    opt->setModifiedFileComponent( m_modifiedFileComponent->text() );
    opt->setOriginalFileComponent( m_originalFileComponent->text() );
    opt->setAutoStackNewFiles( m_autoStackNewFiles->isChecked() );
    opt->setCopyFileComponent( m_copyFileComponent->text() );
    opt->setCopyFileReplacementComponent( m_copyFileReplacementComponent->text() );
}
// vi:expandtab:tabstop=4 shiftwidth=4:

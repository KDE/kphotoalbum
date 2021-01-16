/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "FileVersionDetectionPage.h"

#include <kpabase/SettingsData.h>

#include <KComboBox>
#include <KLocalizedString>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QVBoxLayout>

Settings::FileVersionDetectionPage::FileVersionDetectionPage(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *scrollWidget = new QWidget;
    auto *topLayout = new QVBoxLayout(scrollWidget);

    QString txt;

    // General file searching
    {
        QGroupBox *generalBox = new QGroupBox(i18n("New File Searches"), this);
        topLayout->addWidget(generalBox);
        QVBoxLayout *layout = new QVBoxLayout(generalBox);

        // Search for images on startup
        m_searchForImagesOnStart = new QCheckBox(i18n("Search for new images and videos on startup"), generalBox);
        layout->addWidget(m_searchForImagesOnStart);

        m_ignoreFileExtension = new QCheckBox(i18n("Ignore file extensions when searching for new images and videos"), generalBox);
        layout->addWidget(m_ignoreFileExtension);

        m_skipSymlinks = new QCheckBox(i18n("Skip symbolic links when searching for new images"), generalBox);
        layout->addWidget(m_skipSymlinks);

        m_skipRawIfOtherMatches = new QCheckBox(i18n("Do not read RAW files if a matching JPEG/TIFF file exists"), generalBox);
        layout->addWidget(m_skipRawIfOtherMatches);

        // Exclude directories from search
        QLabel *excludeDirectoriesLabel = new QLabel(i18n("Directories to exclude from new file search:"), generalBox);
        layout->addWidget(excludeDirectoriesLabel);

        m_excludeDirectories = new QLineEdit(generalBox);
        layout->addWidget(m_excludeDirectories);
        excludeDirectoriesLabel->setBuddy(m_excludeDirectories);

        txt = i18n("<p>Specify a comma-separated list of directory names to ignore.</p>"
                   "<p>For example, specifying \"<tt>.thumbs,.thumbnails</tt>\" here will cause "
                   "KPhotoAlbum to ignore these two directory names in any directory when searching for new images.</p>");
        m_excludeDirectories->setWhatsThis(txt);

        txt = i18n("<p>KPhotoAlbum is capable of searching for new images and videos when started, this does, "
                   "however, take some time, so instead you may wish to manually tell KPhotoAlbum to search for new images "
                   "using <b>Maintenance->Rescan for new images</b></p>");
        m_searchForImagesOnStart->setWhatsThis(txt);

        txt = i18n("<p>KPhotoAlbum will normally search new images and videos by their file extension. "
                   "If this option is set, <em>all</em> files neither in the database nor in the block list "
                   "will be checked by their Mime type, regardless of their extension. This will take "
                   "significantly longer than finding files by extension!</p>");
        m_ignoreFileExtension->setWhatsThis(txt);

        txt = i18n("<p>KPhotoAlbum attempts to read all image files whether actual files or symbolic links. If you "
                   "wish to ignore symbolic links, check this option. This is useful if for some reason you have e.g. "
                   "both the original files and symbolic links to these files within your image directory.</p>");
        m_skipSymlinks->setWhatsThis(txt);

        txt = i18n("<p>KPhotoAlbum is capable of reading certain kinds of RAW images. "
                   "Some cameras store both a RAW image and a matching JPEG or TIFF image. "
                   "This causes duplicate images to be stored in KPhotoAlbum, which may be undesirable. "
                   "If this option is checked, KPhotoAlbum will not read RAW files for which matching image files also exist.</p>");
        m_skipRawIfOtherMatches->setWhatsThis(txt);

        txt = i18n("<p>Directories defined here (separated by comma <tt>,</tt>) are "
                   "skipped when searching for new photos. Thumbnail directories of different "
                   "tools should be configured here. E.g. <tt>xml,ThumbNails,.thumbs,.thumbnails</tt>.</p>");
        excludeDirectoriesLabel->setWhatsThis(txt);
    }

    // Original/Modified File Support
    {
        QGroupBox *modifiedBox = new QGroupBox(i18n("File Version Detection Settings"), this);
        topLayout->addWidget(modifiedBox);
        QVBoxLayout *layout = new QVBoxLayout(modifiedBox);

        m_detectModifiedFiles = new QCheckBox(i18n("Try to detect multiple versions of files"), modifiedBox);
        layout->addWidget(m_detectModifiedFiles);

        QLabel *modifiedFileComponentLabel = new QLabel(i18n("File versions search regexp:"), modifiedBox);
        layout->addWidget(modifiedFileComponentLabel);

        m_modifiedFileComponent = new QLineEdit(modifiedBox);
        layout->addWidget(m_modifiedFileComponent);

        QLabel *originalFileComponentLabel = new QLabel(i18n("Original file replacement text:"), modifiedBox);
        layout->addWidget(originalFileComponentLabel);

        m_originalFileComponent = new QLineEdit(modifiedBox);
        layout->addWidget(m_originalFileComponent);

        m_moveOriginalContents = new QCheckBox(i18n("Move meta-data (i.e. delete tags from the original)"), modifiedBox);
        layout->addWidget(m_moveOriginalContents);

        m_autoStackNewFiles = new QCheckBox(i18n("Automatically stack new versions of images"), modifiedBox);
        layout->addWidget(m_autoStackNewFiles);

        txt = i18n("<p>When KPhotoAlbum searches for new files and finds a file that matches the "
                   "<i>modified file search regexp</i> it is assumed that an original version of "
                   "the image may exist. The regexp pattern will be replaced with the <i>original "
                   "file replacement text</i> and if that file exists, all associated metadata (category "
                   "information, ratings, etc) will be copied or moved from the original file to the new one.</p>");
        m_detectModifiedFiles->setWhatsThis(txt);

        txt = i18n("<p>A perl regular expression that should match a modified file. "
                   "<ul><li>A dot matches a single character (<tt>\\.</tt> matches a dot)</li> "
                   "  <li>You can use the quantifiers <tt>*</tt>,<tt>+</tt>,<tt>?</tt>, or you can "
                   "    match multiple occurrences of an expression by using curly brackets (e.g. "
                   "<tt>e{0,1}</tt> matches 0 or 1 occurrences of the character \"e\").</li> "
                   "  <li>You can group parts of the expression using parenthesis.</li> "
                   "</ul>Example: <tt>-modified\\.(jpg|tiff)</tt> </p>");
        modifiedFileComponentLabel->setWhatsThis(txt);
        m_modifiedFileComponent->setWhatsThis(txt);

        txt = i18n("<p>A string that is used to replace the match from the <i>File versions search regexp</i>. "
                   "This can be a semicolon (<tt>;</tt>) separated list. Each string is used to replace the match "
                   "in the new file's name until an original file is found or we run out of options.</p>");
        originalFileComponentLabel->setWhatsThis(txt);
        m_originalFileComponent->setWhatsThis(txt);

        txt = i18n("<p>The tagging is moved from the original file to the new file. This way "
                   "only the latest version of an image is tagged.</p>");
        m_moveOriginalContents->setWhatsThis(txt);

        txt = i18n("<p>If this option is set, new versions of an image are automatically stacked "
                   "and placed to the top of the stack. This way the new image is shown when the "
                   "stack is in collapsed state - the default state in KPhotoAlbum.</p>");
        m_autoStackNewFiles->setWhatsThis(txt);
    }

    // Copy File Support
    {
        QGroupBox *copyBox = new QGroupBox(i18nc("Configure the feature to make a copy of a file first and then open the copied file with an external application", "Copy File and Open with an External Application"), this);
        topLayout->addWidget(copyBox);
        QVBoxLayout *layout = new QVBoxLayout(copyBox);

        QLabel *copyFileComponentLabel = new QLabel(i18n("Copy file search regexp:"), copyBox);
        layout->addWidget(copyFileComponentLabel);

        m_copyFileComponent = new QLineEdit(copyBox);
        layout->addWidget(m_copyFileComponent);

        QLabel *copyFileReplacementComponentLabel = new QLabel(i18n("Copy file replacement text:"), copyBox);
        layout->addWidget(copyFileReplacementComponentLabel);

        m_copyFileReplacementComponent = new QLineEdit(copyBox);
        layout->addWidget(m_copyFileReplacementComponent);

        txt = i18n("<p>KPhotoAlbum can make a copy of an image before opening it with an external application. This configuration defines how the new file is named.</p>"
                   "<p>The regular expression defines the part of the original file name that is replaced with the <i>replacement text</i>. "
                   "E.g. regexp <i>\"\\.(jpg|png)\"</i> and replacement text <i>\"-mod.\\1\"</i> would copy test.jpg to test-mod.jpg and open the new file in selected application.</p>");
        copyFileComponentLabel->setWhatsThis(txt);
        m_copyFileComponent->setWhatsThis(txt);
        copyFileReplacementComponentLabel->setWhatsThis(txt);
        m_copyFileReplacementComponent->setWhatsThis(txt);
    }

    // Loader Optimization Setting (prototype)
    {
        QGroupBox *loadOptimizationBox = new QGroupBox(i18n("EXPERIMENTAL: Tune new image loading for best performance."), this);
        topLayout->addWidget(loadOptimizationBox);
        QGridLayout *layout = new QGridLayout(loadOptimizationBox);
        int row = 0;

        // Loader preset
        QLabel *loadOptimizationPresetLabel = new QLabel(i18n("Type of media on which your images are stored."));
        layout->addWidget(loadOptimizationPresetLabel, row, 0);
        layout->setColumnStretch(0, 1);

        m_loadOptimizationPreset = new KComboBox;
        m_loadOptimizationPreset->addItems(QStringList() << i18n("Hard Disk")
                                                         << i18n("Network")
                                                         << i18n("SATA SSD")
                                                         << i18n("Slow PCIe/NVMe")
                                                         << i18n("Fast PCIe/NVMe")
                                                         << i18n("Manual Settings")); // manual is expected to be the last item
        layout->addWidget(m_loadOptimizationPreset, row, 1);

        txt = i18n("<p>Tune image loading for best performance based on the type of storage your image database resides on.  If your image database resides on multiple media, choose the slowest media type used.</p>"
                   "<p>Use Manual Settings to configure details of how the loading is performed.</p>");
        loadOptimizationPresetLabel->setWhatsThis(txt);
        m_loadOptimizationPreset->setWhatsThis(txt);
        connect(m_loadOptimizationPreset, QOverload<int>::of(&QComboBox::activated), this, &FileVersionDetectionPage::slotUpdateOptimizationUI);

        // Overlap load with MD5 computation
        ++row;
        QLabel *overlapLoadMD5Label = new QLabel(i18n("Calculate MD5 checksum while images are being preloaded"));
        layout->addWidget(overlapLoadMD5Label, row, 0);
        m_overlapLoadMD5 = new QCheckBox;
        layout->addWidget(m_overlapLoadMD5, row, 1);

        txt = i18n("<p>Calculate MD5 checksums while images are being preloaded.  This works well if the storage is very fast, such as an NVMe drive.  If the storage is slow, this degrades performance as the checksum calculation has to wait longer for the images to be preloaded.</p>");
        overlapLoadMD5Label->setWhatsThis(txt);
        m_overlapLoadMD5->setWhatsThis(txt);

        // Threads for preload
        ++row;
        QLabel *preloadThreadCountLabel = new QLabel(i18n("Number of threads to use for preloading (scouting) images"));
        layout->addWidget(preloadThreadCountLabel, row, 0);

        m_preloadThreadCount = new QSpinBox;
        m_preloadThreadCount->setRange(1, 16);
        m_preloadThreadCount->setSingleStep(1);
        layout->addWidget(m_preloadThreadCount, row, 1);

        txt = i18n("<p>Number of threads to use for preloading images to have them in memory when their checksums are calculated.  This should generally be set higher for faster storage, but not more than the number of cores in your CPU. Default is 1, which works well for mechanical hard disks.</p>");
        preloadThreadCountLabel->setWhatsThis(txt);
        m_preloadThreadCount->setWhatsThis(txt);

        // Threads for thumbnailPreload
        ++row;
        QLabel *thumbnailPreloadThreadCountLabel = new QLabel(i18n("Number of threads to use for thumbnailPreloading (scouting) images"));
        layout->addWidget(thumbnailPreloadThreadCountLabel, row, 0);

        m_thumbnailPreloadThreadCount = new QSpinBox;
        m_thumbnailPreloadThreadCount->setRange(1, 16);
        m_thumbnailPreloadThreadCount->setSingleStep(1);
        layout->addWidget(m_thumbnailPreloadThreadCount, row, 1);

        txt = i18n("<p>Number of threads to use for preloading images prior to building thumbnails.  Normally this should be set to 1; the exception might be if you have very fast storage.</p>");
        thumbnailPreloadThreadCountLabel->setWhatsThis(txt);
        m_thumbnailPreloadThreadCount->setWhatsThis(txt);

        // Threads for thumbnailBuilder
        ++row;
        QLabel *thumbnailBuilderThreadCountLabel = new QLabel(i18n("Number of threads to use for building thumbnails"));
        layout->addWidget(thumbnailBuilderThreadCountLabel, row, 0);

        m_thumbnailBuilderThreadCount = new QSpinBox(loadOptimizationBox);
        m_thumbnailBuilderThreadCount->setRange(0, 16);
        m_thumbnailBuilderThreadCount->setSingleStep(1);
        layout->addWidget(m_thumbnailBuilderThreadCount, row, 1);

        txt = i18n("<p>Number of threads to use for building thumbnails.  If set to 0 this will be set automatically one less than the number of cores, at least one and no more than three.  If you have fast storage and a CPU with many cores, you may see benefit from setting this to a larger value.</p>"
                   "<p>KPhotoAlbum must be restarted for changes to take effect.</p>");
        thumbnailBuilderThreadCountLabel->setWhatsThis(txt);
        m_thumbnailBuilderThreadCount->setWhatsThis(txt);
    }

    auto *scrollArea = new QScrollArea;
    mainLayout->addWidget(scrollArea);
    scrollArea->setWidget(scrollWidget);
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
    delete m_loadOptimizationPreset;
    delete m_overlapLoadMD5;
    delete m_preloadThreadCount;
    delete m_thumbnailPreloadThreadCount;
    delete m_thumbnailBuilderThreadCount;
}

void Settings::FileVersionDetectionPage::loadSettings(Settings::SettingsData *opt)
{
    m_searchForImagesOnStart->setChecked(opt->searchForImagesOnStart());
    m_ignoreFileExtension->setChecked(opt->ignoreFileExtension());
    m_skipSymlinks->setChecked(opt->skipSymlinks());
    m_skipRawIfOtherMatches->setChecked(opt->skipRawIfOtherMatches());
    m_excludeDirectories->setText(opt->excludeDirectories());
    m_detectModifiedFiles->setChecked(opt->detectModifiedFiles());
    m_modifiedFileComponent->setText(opt->modifiedFileComponent());
    m_originalFileComponent->setText(opt->originalFileComponent());
    m_moveOriginalContents->setChecked(opt->moveOriginalContents());
    m_autoStackNewFiles->setChecked(opt->autoStackNewFiles());
    m_copyFileComponent->setText(opt->copyFileComponent());
    m_copyFileReplacementComponent->setText(opt->copyFileReplacementComponent());
    m_loadOptimizationPreset->setCurrentIndex(opt->loadOptimizationPreset());
    m_overlapLoadMD5->setChecked(opt->overlapLoadMD5());
    m_preloadThreadCount->setValue(opt->preloadThreadCount());
    m_thumbnailPreloadThreadCount->setValue(opt->thumbnailPreloadThreadCount());
    m_thumbnailBuilderThreadCount->setValue(opt->thumbnailBuilderThreadCount());
    slotUpdateOptimizationUI();
}

void Settings::FileVersionDetectionPage::saveSettings(Settings::SettingsData *opt)
{
    opt->setSearchForImagesOnStart(m_searchForImagesOnStart->isChecked());
    opt->setIgnoreFileExtension(m_ignoreFileExtension->isChecked());
    opt->setSkipSymlinks(m_skipSymlinks->isChecked());
    opt->setSkipRawIfOtherMatches(m_skipRawIfOtherMatches->isChecked());
    opt->setExcludeDirectories(m_excludeDirectories->text());
    opt->setDetectModifiedFiles(m_detectModifiedFiles->isChecked());
    opt->setModifiedFileComponent(m_modifiedFileComponent->text());
    opt->setOriginalFileComponent(m_originalFileComponent->text());
    opt->setAutoStackNewFiles(m_autoStackNewFiles->isChecked());
    opt->setCopyFileComponent(m_copyFileComponent->text());
    opt->setCopyFileReplacementComponent(m_copyFileReplacementComponent->text());
    opt->setLoadOptimizationPreset(m_loadOptimizationPreset->currentIndex());
    opt->setOverlapLoadMD5(m_overlapLoadMD5->isChecked());
    opt->setPreloadThreadCount(m_preloadThreadCount->value());
    opt->setThumbnailPreloadThreadCount(m_thumbnailPreloadThreadCount->value());
    opt->setThumbnailBuilderThreadCount(m_thumbnailBuilderThreadCount->value());
}

void Settings::FileVersionDetectionPage::slotUpdateOptimizationUI()
{
    const bool manualModeIsSelected = (m_loadOptimizationPreset->currentIndex() + 1 == m_loadOptimizationPreset->count());
    m_overlapLoadMD5->setEnabled(manualModeIsSelected);
    m_preloadThreadCount->setEnabled(manualModeIsSelected);
    m_thumbnailPreloadThreadCount->setEnabled(manualModeIsSelected);
    m_thumbnailBuilderThreadCount->setEnabled(manualModeIsSelected);
}
// vi:expandtab:tabstop=4 shiftwidth=4:

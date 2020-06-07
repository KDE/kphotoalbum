/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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

#include "ImportDialog.h"

#include "ImageRow.h"
#include "ImportMatcher.h"
#include "KimFileReader.h"
#include "MD5CheckPage.h"

#include <DB/ImageInfo.h>
#include <Settings/SettingsData.h>
#include <XMLDB/Database.h>

#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>

using Utilities::StringSet;

class QPushButton;
using namespace ImportExport;

ImportDialog::ImportDialog(QWidget *parent)
    : KAssistantDialog(parent)
    , m_hasFilled(false)
    , m_md5CheckPage(nullptr)
{
}

bool ImportDialog::exec(KimFileReader *kimFileReader, const QUrl &kimFileURL)
{
    m_kimFileReader = kimFileReader;

    if (kimFileURL.isLocalFile()) {
        QDir cwd;
        // convert relative local path to absolute
        m_kimFile = QUrl::fromLocalFile(cwd.absoluteFilePath(kimFileURL.toLocalFile()))
                        .adjusted(QUrl::NormalizePathSegments);
    } else {
        m_kimFile = kimFileURL;
    }

    QByteArray indexXML = m_kimFileReader->indexXML();
    if (indexXML.isNull())
        return false;

    bool ok = readFile(indexXML);
    if (!ok)
        return false;

    setupPages();

    return KAssistantDialog::exec();
}

bool ImportDialog::readFile(const QByteArray &data)
{
    XMLDB::ReaderPtr reader = XMLDB::ReaderPtr(new XMLDB::XmlReader(DB::ImageDB::instance()->uiDelegate(),
                                                                    m_kimFile.toDisplayString()));
    reader->addData(data);

    XMLDB::ElementInfo info = reader->readNextStartOrStopElement(QString::fromUtf8("KimDaBa-export"));
    if (!info.isStartToken)
        reader->complainStartElementExpected(QString::fromUtf8("KimDaBa-export"));

    // Read source
    QString source = reader->attribute(QString::fromUtf8("location")).toLower();
    if (source != QString::fromLatin1("inline") && source != QString::fromLatin1("external")) {
        KMessageBox::error(this, i18n("<p>XML file did not specify the source of the images, "
                                      "this is a strong indication that the file is corrupted</p>"));
        return false;
    }

    m_externalSource = (source == QString::fromLatin1("external"));

    // Read base url
    m_baseUrl = QUrl::fromUserInput(reader->attribute(QString::fromLatin1("baseurl")));

    while (reader->readNextStartOrStopElement(QString::fromUtf8("image")).isStartToken) {
        const DB::FileName fileName = DB::FileName::fromRelativePath(reader->attribute(QString::fromUtf8("file")));
        DB::ImageInfoPtr info = XMLDB::Database::createImageInfo(fileName, reader);
        m_images.append(info);
    }
    // the while loop already read the end element, so we tell readEndElement to not read the next token:
    reader->readEndElement(false);

    return true;
}

void ImportDialog::setupPages()
{
    createIntroduction();
    createImagesPage();
    createDestination();
    createCategoryPages();
    connect(this, &ImportDialog::currentPageChanged, this, &ImportDialog::updateNextButtonState);
    QPushButton *helpButton = buttonBox()->button(QDialogButtonBox::Help);
    connect(helpButton, &QPushButton::clicked, this, &ImportDialog::slotHelp);
}

void ImportDialog::createIntroduction()
{
    QString txt = i18n("<h1><font size=\"+2\">Welcome to KPhotoAlbum Import</font></h1>"
                       "This wizard will take you through the steps of an import operation. The steps are: "
                       "<ol><li>First you must select which images you want to import from the export file. "
                       "You do so by selecting the checkbox next to the image.</li>"
                       "<li>Next you must tell KPhotoAlbum in which directory to put the images. This directory must "
                       "of course be below the directory root KPhotoAlbum uses for images. "
                       "KPhotoAlbum will take care to avoid name clashes</li>"
                       "<li>The next step is to specify which categories you want to import (People, Places, ... ) "
                       "and also tell KPhotoAlbum how to match the categories from the file to your categories. "
                       "Imagine you load from a file, where a category is called <b>Blomst</b> (which is the "
                       "Danish word for flower), then you would likely want to match this with your category, which might be "
                       "called <b>Blume</b> (which is the German word for flower) - of course given you are German.</li>"
                       "<li>The final steps, is matching the individual tokens from the categories. I may call myself <b>Jesper</b> "
                       "in my image database, while you want to call me by my full name, namely <b>Jesper K. Pedersen</b>. "
                       "In this step non matches will be highlighted in red, so you can see which tokens was not found in your "
                       "database, or which tokens was only a partial match.</li></ol>");

    QLabel *intro = new QLabel(txt, this);
    intro->setWordWrap(true);
    addPage(intro, i18nc("@title:tab introduction page", "Introduction"));
}

void ImportDialog::createImagesPage()
{
    QScrollArea *top = new QScrollArea;
    top->setWidgetResizable(true);

    QWidget *container = new QWidget;
    QVBoxLayout *lay1 = new QVBoxLayout(container);
    top->setWidget(container);

    // Select all and Deselect All buttons
    QHBoxLayout *lay2 = new QHBoxLayout;
    lay1->addLayout(lay2);

    QPushButton *selectAll = new QPushButton(i18n("Select All"), container);
    lay2->addWidget(selectAll);
    QPushButton *selectNone = new QPushButton(i18n("Deselect All"), container);
    lay2->addWidget(selectNone);
    lay2->addStretch(1);
    connect(selectAll, &QPushButton::clicked, this, &ImportDialog::slotSelectAll);
    connect(selectNone, &QPushButton::clicked, this, &ImportDialog::slotSelectNone);

    QGridLayout *lay3 = new QGridLayout;
    lay1->addLayout(lay3);

    lay3->setColumnStretch(2, 1);

    int row = 0;
    for (DB::ImageInfoListConstIterator it = m_images.constBegin(); it != m_images.constEnd(); ++it, ++row) {
        DB::ImageInfoPtr info = *it;
        ImageRow *ir = new ImageRow(info, this, m_kimFileReader, container);
        lay3->addWidget(ir->m_checkbox, row, 0);

        QPixmap pixmap = m_kimFileReader->loadThumbnail(info->fileName().relative());
        if (!pixmap.isNull()) {
            QPushButton *but = new QPushButton(container);
            but->setIcon(pixmap);
            but->setIconSize(pixmap.size());
            lay3->addWidget(but, row, 1);
            connect(but, &QPushButton::clicked, ir, &ImageRow::showImage);
        } else {
            QLabel *label = new QLabel(info->label());
            lay3->addWidget(label, row, 1);
        }

        QLabel *label = new QLabel(QString::fromLatin1("<p>%1</p>").arg(info->description()));
        lay3->addWidget(label, row, 2);
        m_imagesSelect.append(ir);
    }

    addPage(top, i18n("Select Which Images to Import"));
}

void ImportDialog::createDestination()
{
    QWidget *top = new QWidget(this);
    QVBoxLayout *topLay = new QVBoxLayout(top);
    QHBoxLayout *lay = new QHBoxLayout;
    topLay->addLayout(lay);

    topLay->addStretch(1);

    QLabel *label = new QLabel(i18n("Destination of images: "), top);
    lay->addWidget(label);

    m_destinationEdit = new QLineEdit(top);
    lay->addWidget(m_destinationEdit, 1);

    QPushButton *but = new QPushButton(QString::fromLatin1("..."), top);
    but->setFixedWidth(30);
    lay->addWidget(but);

    m_destinationEdit->setText(Settings::SettingsData::instance()->imageDirectory());
    connect(but, &QPushButton::clicked, this, &ImportDialog::slotEditDestination);
    connect(m_destinationEdit, &QLineEdit::textChanged, this, &ImportDialog::updateNextButtonState);
    m_destinationPage = addPage(top, i18n("Destination of Images"));
}

void ImportDialog::slotEditDestination()
{
    QString file = QFileDialog::getExistingDirectory(this, QString(), m_destinationEdit->text());
    if (!file.isNull()) {
        if (!QFileInfo(file).absoluteFilePath().startsWith(QFileInfo(Settings::SettingsData::instance()->imageDirectory()).absoluteFilePath())) {
            KMessageBox::error(this, i18n("The directory must be a subdirectory of %1", Settings::SettingsData::instance()->imageDirectory()));
        } else if (QFileInfo(file).absoluteFilePath().startsWith(
                       QFileInfo(Settings::SettingsData::instance()->imageDirectory()).absoluteFilePath() + QString::fromLatin1("CategoryImages"))) {
            KMessageBox::error(this, i18n("This directory is reserved for category images."));
        } else {
            m_destinationEdit->setText(file);
            updateNextButtonState();
        }
    }
}

void ImportDialog::updateNextButtonState()
{
    bool enabled = true;
    if (currentPage() == m_destinationPage) {
        QString dest = m_destinationEdit->text();
        if (QFileInfo(dest).isFile())
            enabled = false;
        else if (!QFileInfo(dest).absoluteFilePath().startsWith(QFileInfo(Settings::SettingsData::instance()->imageDirectory()).absoluteFilePath()))
            enabled = false;
    }

    setValid(currentPage(), enabled);
}

void ImportDialog::createCategoryPages()
{
    QStringList categories;
    const DB::ImageInfoList images = selectedImages();
    for (DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it) {
        const DB::ImageInfoPtr info = *it;
        const QStringList categoriesForImage = info->availableCategories();
        for (const QString &category : categoriesForImage) {
            auto catPtr = DB::ImageDB::instance()->categoryCollection()->categoryForName(category);
            if (!categories.contains(category) && !(catPtr && catPtr->isSpecialCategory())) {
                categories.append(category);
            }
        }
    }

    if (!categories.isEmpty()) {
        m_categoryMatcher = new ImportMatcher(QString(), QString(), categories, DB::ImageDB::instance()->categoryCollection()->categoryNames(DB::CategoryCollection::IncludeSpecialCategories::No),
                                              false, this);
        m_categoryMatcherPage = addPage(m_categoryMatcher, i18n("Match Categories"));

        QWidget *dummy = new QWidget;
        m_dummy = addPage(dummy, QString());
    } else {
        m_categoryMatcherPage = nullptr;
        possiblyAddMD5CheckPage();
    }
}

ImportMatcher *ImportDialog::createCategoryPage(const QString &myCategory, const QString &otherCategory)
{
    StringSet otherItems;
    DB::ImageInfoList images = selectedImages();
    for (DB::ImageInfoListConstIterator it = images.constBegin(); it != images.constEnd(); ++it) {
        otherItems += (*it)->itemsOfCategory(otherCategory);
    }

    QStringList myItems = DB::ImageDB::instance()->categoryCollection()->categoryForName(myCategory)->itemsInclCategories();
    myItems.sort();

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const QStringList otherItemsList(otherItems.begin(), otherItems.end());
#else
    const QStringList otherItemsList = otherItems.toList();
#endif
    ImportMatcher *matcher = new ImportMatcher(otherCategory, myCategory, otherItemsList, myItems, true, this);
    addPage(matcher, myCategory);
    return matcher;
}

void ImportDialog::next()
{
    if (currentPage() == m_destinationPage) {
        QString dir = m_destinationEdit->text();
        if (!QFileInfo(dir).exists()) {
            int answer = KMessageBox::questionYesNo(this, i18n("Directory %1 does not exist. Should it be created?", dir));
            if (answer == KMessageBox::Yes) {
                bool ok = QDir().mkpath(dir);
                if (!ok) {
                    KMessageBox::error(this, i18n("Error creating directory %1", dir));
                    return;
                }
            } else
                return;
        }
    }
    if (!m_hasFilled && currentPage() == m_categoryMatcherPage) {
        m_hasFilled = true;
        m_categoryMatcher->setEnabled(false);
        removePage(m_dummy);

        ImportMatcher *matcher = nullptr;
        const auto matchers = m_categoryMatcher->m_matchers;
        for (const CategoryMatch *match : matchers) {
            if (match->m_checkbox->isChecked()) {
                matcher = createCategoryPage(match->m_combobox->currentText(), match->m_text);
                m_matchers.append(matcher);
            }
        }
        possiblyAddMD5CheckPage();
    }

    KAssistantDialog::next();
}

void ImportDialog::slotSelectAll()
{
    selectImage(true);
}

void ImportDialog::slotSelectNone()
{
    selectImage(false);
}

void ImportDialog::selectImage(bool on)
{
    for (ImageRow *row : qAsConst(m_imagesSelect)) {
        row->m_checkbox->setChecked(on);
    }
}

DB::ImageInfoList ImportDialog::selectedImages() const
{
    DB::ImageInfoList res;
    for (QList<ImageRow *>::ConstIterator it = m_imagesSelect.begin(); it != m_imagesSelect.end(); ++it) {
        if ((*it)->m_checkbox->isChecked())
            res.append((*it)->m_info);
    }
    return res;
}

void ImportDialog::slotHelp()
{
    KHelpClient::invokeHelp(QString::fromLatin1("chp-importExport"));
}

ImportSettings ImportExport::ImportDialog::settings()
{
    ImportSettings settings;
    settings.setSelectedImages(selectedImages());
    settings.setDestination(m_destinationEdit->text());
    settings.setExternalSource(m_externalSource);
    settings.setKimFile(m_kimFile);
    settings.setBaseURL(m_baseUrl);

    if (m_md5CheckPage) {
        settings.setImportActions(m_md5CheckPage->settings());
    }

    for (ImportMatcher *match : m_matchers)
        settings.addCategoryMatchSetting(match->settings());

    return settings;
}

void ImportExport::ImportDialog::possiblyAddMD5CheckPage()
{
    if (MD5CheckPage::pageNeeded(settings())) {
        m_md5CheckPage = new MD5CheckPage(settings());
        addPage(m_md5CheckPage, i18n("How to resolve clashes"));
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

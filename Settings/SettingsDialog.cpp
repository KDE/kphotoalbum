// SPDX-FileCopyrightText: 2003-2022 The KPhotoAlbum Development Team
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "SettingsDialog.h"

#include "BirthdayPage.h"
#include "CategoryPage.h"
#include "DatabaseBackendPage.h"
#include "ExifPage.h"
#include "FileVersionDetectionPage.h"
#include "GeneralPage.h"
#include "TagGroupsPage.h"
#include "ThumbnailsPage.h"
#include "ViewerPage.h"

#include <Utilities/ShowBusyCursor.h>

#include <KLocalizedString>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

struct Data {
    Settings::SettingsPage page;
    QString title;
    const char *icon;
    QWidget *widget;
};

Settings::SettingsDialog::SettingsDialog(QWidget *parent)
    : KPageDialog(parent)
{
    m_generalPage = new Settings::GeneralPage(this);
    m_fileVersionDetectionPage = new Settings::FileVersionDetectionPage(this);
    m_thumbnailsPage = new Settings::ThumbnailsPage(this);
    m_categoryPage = new Settings::CategoryPage(this);
    m_tagGroupsPage = new Settings::TagGroupsPage(this);
    m_viewerPage = new Settings::ViewerPage(this);
    m_exifPage = new Settings::ExifPage(this);
    m_birthdayPage = new Settings::BirthdayPage(this);
    m_databaseBackendPage = new Settings::DatabaseBackendPage(this);

    Data data[] = {
        { SettingsPage::GeneralPage, i18n("General"), "configure-shortcuts", m_generalPage },
        { SettingsPage::FileVersionDetectionPage, i18n("File Searching & Versions"), "system-search", m_fileVersionDetectionPage },
        { SettingsPage::ThumbnailsPage, i18n("Thumbnail View"), "view-preview", m_thumbnailsPage },
        { SettingsPage::CategoryPage, i18n("Categories"), "edit-group", m_categoryPage },
        { SettingsPage::BirthdayPage, i18n("Birthdays"), "view-calendar-birthday", m_birthdayPage },
        { SettingsPage::TagGroupsPage, i18n("Tag Groups"), "view-group", m_tagGroupsPage },
        { SettingsPage::ViewerPage, i18n("Viewer"), "document-preview", m_viewerPage },
        { SettingsPage::ExifPage, i18n("Exif/IPTC Information"), "document-properties", m_exifPage },
        { SettingsPage::DatabaseBackendPage, i18n("Database Backend"), "document-save", m_databaseBackendPage },
        { SettingsPage::GeneralPage, QString(), "", 0 }
    };

    int i = 0;
    while (data[i].widget != 0) {
        KPageWidgetItem *page = new KPageWidgetItem(data[i].widget, data[i].title);
        page->setHeader(data[i].title);
        page->setIcon(QIcon::fromTheme(QString::fromLatin1(data[i].icon)));
        addPage(page);
        m_pages[data[i].page] = page;
        ++i;
    }

    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(this, &QDialog::accepted, this, &SettingsDialog::slotMyOK);
    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &SettingsDialog::slotMyOK);
    connect(this, &QDialog::rejected, m_birthdayPage, &Settings::BirthdayPage::discardChanges);

    setWindowTitle(i18nc("@title:window", "Settings"));

    connect(m_categoryPage, &Settings::CategoryPage::categoryChangesPending, m_tagGroupsPage, &Settings::TagGroupsPage::categoryChangesPending);
    connect(this, &SettingsDialog::currentPageChanged, m_tagGroupsPage, &Settings::TagGroupsPage::slotPageChange);
    connect(this, &SettingsDialog::currentPageChanged, m_birthdayPage, &Settings::BirthdayPage::pageChange);

    connect(this, &SettingsDialog::rejected, m_categoryPage, &Settings::CategoryPage::resetCategoryLabel);
}

void Settings::SettingsDialog::show()
{
    Settings::SettingsData *opt = Settings::SettingsData::instance();

    m_generalPage->loadSettings(opt);
    m_fileVersionDetectionPage->loadSettings(opt);
    m_thumbnailsPage->loadSettings(opt);
    m_tagGroupsPage->loadSettings();
    m_databaseBackendPage->loadSettings(opt);
    m_viewerPage->loadSettings(opt);
    m_categoryPage->loadSettings(opt);
    m_exifPage->loadSettings(opt);
    m_categoryPage->enableDisable(false);
    m_birthdayPage->reload();
    m_categoryPage->resetCategoryNamesChanged();

    QDialog::show();
}

void Settings::SettingsDialog::activatePage(Settings::SettingsPage pageId)
{
    auto page = m_pages.value(pageId, nullptr);
    if (page)
        setCurrentPage(page);
}

// QDialog has a slotOK which we do not want to override.
void Settings::SettingsDialog::slotMyOK()
{
    Utilities::ShowBusyCursor dummy;
    Settings::SettingsData *opt = Settings::SettingsData::instance();

    m_categoryPage->resetInterface();
    m_generalPage->saveSettings(opt);
    m_fileVersionDetectionPage->saveSettings(opt);
    m_thumbnailsPage->saveSettings(opt);
    m_birthdayPage->saveSettings();
    m_tagGroupsPage->saveSettings();
    m_categoryPage->saveSettings(opt, m_tagGroupsPage->memberMap());
    m_viewerPage->saveSettings(opt);
    m_exifPage->saveSettings(opt);
    m_databaseBackendPage->saveSettings(opt);

    Q_EMIT changed();
    KSharedConfig::openConfig()->sync();
}

void Settings::SettingsDialog::keyPressEvent(QKeyEvent *)
{
    // This prevents the dialog to be closed if the ENTER key is pressed anywhere
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_SettingsDialog.cpp"

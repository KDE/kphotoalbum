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

#include "config-kpa-kipi.h"
#include "config-kpa-kface.h"
#include "SettingsDialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KSharedConfig>

#include "BirthdayPage.h"
#include "CategoryPage.h"
#include "DatabaseBackendPage.h"
#include "ExifPage.h"
#ifdef HAVE_KFACE
#include "FaceManagementPage.h"
#endif
#include "FileVersionDetectionPage.h"
#include "GeneralPage.h"
#include "PluginsPage.h"
#include "TagGroupsPage.h"
#include "ThumbnailsPage.h"
#include "ViewerPage.h"
#include <Utilities/ShowBusyCursor.h>

struct Data
{
    QString title;
    const char* icon;
    QWidget* widget;
};

Settings::SettingsDialog::SettingsDialog( QWidget* parent)
     :KPageDialog( parent )
{
    m_generalPage = new Settings::GeneralPage(this);
    m_fileVersionDetectionPage = new Settings::FileVersionDetectionPage(this);
    m_thumbnailsPage = new Settings::ThumbnailsPage(this);
    m_categoryPage = new Settings::CategoryPage(this);
    m_tagGroupsPage = new Settings::TagGroupsPage(this);
    m_viewerPage = new Settings::ViewerPage(this);

#ifdef HASKIPI
    m_pluginsPage = new Settings::PluginsPage(this);
#endif

#ifdef HAVE_EXIV2
    m_exifPage = new Settings::ExifPage(this);
#endif

#ifdef HAVE_KFACE
    m_faceManagementPage = new Settings::FaceManagementPage(this);
#endif

    m_birthdayPage = new Settings::BirthdayPage(this);

    m_databaseBackendPage = new Settings::DatabaseBackendPage(this);


    Data data[] = {
        { i18n("General"), "configure-shortcuts", m_generalPage },
        { i18n("File Searching & Versions"), "system-search", m_fileVersionDetectionPage },
        { i18n("Thumbnail View" ), "view-preview", m_thumbnailsPage },
        { i18n("Categories"), "edit-group", m_categoryPage },
        { i18n("Birthdays"), "view-calendar-birthday", m_birthdayPage },
        { i18n("Tag Groups" ), "view-group", m_tagGroupsPage },
        { i18n("Viewer" ), "document-preview", m_viewerPage },
#ifdef HASKIPI
        { i18n("Plugins" ), "plugins", m_pluginsPage },
#endif

#ifdef HAVE_EXIV2
        { i18n("EXIF/IPTC Information" ), "document-properties", m_exifPage },
#endif
        { i18n("Database backend"), "document-save", m_databaseBackendPage },
#ifdef HAVE_KFACE
        { i18n("Face management" ), "edit-image-face-detect", m_faceManagementPage },
#endif
        { QString(), "", 0 }
    };

    int i = 0;
    while ( data[i].widget != 0 ) {
        KPageWidgetItem* page = new KPageWidgetItem( data[i].widget, data[i].title );
        page->setHeader( data[i].title );
        page->setIcon( QIcon::fromTheme( QString::fromLatin1( data[i].icon ) ) );
        addPage( page );
        ++i;
    }

    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    connect(this, &QDialog::accepted,
            this, &SettingsDialog::slotMyOK);
    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &SettingsDialog::slotMyOK);
    connect(this, &QDialog::rejected, m_birthdayPage, &Settings::BirthdayPage::discardChanges);

    setWindowTitle( i18n( "Settings" ) );

    connect(m_categoryPage, &Settings::CategoryPage::categoryChangesPending,
            m_tagGroupsPage, &Settings::TagGroupsPage::categoryChangesPending);
    connect(this, &SettingsDialog::currentPageChanged,
            m_tagGroupsPage, &Settings::TagGroupsPage::slotPageChange);
#ifdef HAVE_KFACE
    connect(this, &SettingsDialog::currentPageChanged,
            m_faceManagementPage, &Settings::FaceManagementPage::slotPageChange);
#endif
    connect(this, &SettingsDialog::currentPageChanged,
            m_birthdayPage, &Settings::BirthdayPage::pageChange);

    // slot is protected -> use old style connect:
    connect(this, SIGNAL(rejected()),
            m_categoryPage, SLOT(resetCategoryLabel()));
}

void Settings::SettingsDialog::show()
{
    Settings::SettingsData* opt = Settings::SettingsData::instance();

    m_generalPage->loadSettings( opt );
    m_fileVersionDetectionPage->loadSettings( opt );
    m_thumbnailsPage->loadSettings(opt);
    m_tagGroupsPage->loadSettings();
    m_databaseBackendPage->loadSettings(opt);
    m_viewerPage->loadSettings(opt);

#ifdef HASKIPI
    m_pluginsPage->loadSettings(opt);
#endif

    m_categoryPage->loadSettings(opt);

#ifdef HAVE_EXIV2
    m_exifPage->loadSettings( opt );
#endif

#ifdef HAVE_KFACE
    m_faceManagementPage->loadSettings(opt);
#endif

    m_categoryPage->enableDisable( false );

    m_birthdayPage->reload();
    m_categoryPage->resetCategoryNamesChanged();

    QDialog::show();
}

// QDialog has a slotOK which we do not want to override.
void Settings::SettingsDialog::slotMyOK()
{
    Utilities::ShowBusyCursor dummy;
    Settings::SettingsData* opt = Settings::SettingsData::instance();

    m_categoryPage->resetInterface();
    m_generalPage->saveSettings( opt );
    m_fileVersionDetectionPage->saveSettings( opt );
    m_thumbnailsPage->saveSettings(opt);

    m_birthdayPage->saveSettings();
    m_tagGroupsPage->saveSettings();
    m_categoryPage->saveSettings( opt, m_tagGroupsPage->memberMap() );

    m_viewerPage->saveSettings( opt );

#ifdef HASKIPI
    m_pluginsPage->saveSettings( opt );
#endif

#ifdef HAVE_EXIV2
    m_exifPage->saveSettings(opt);
#endif

#ifdef HAVE_KFACE
    m_faceManagementPage->saveSettings(opt);
    m_faceManagementPage->clearDatabaseEntries();
#endif

    m_databaseBackendPage->saveSettings(opt);

    emit changed();
    KSharedConfig::openConfig()->sync();
}

void Settings::SettingsDialog::showBackendPage()
{
    setCurrentPage(m_backendPage);
}

void Settings::SettingsDialog::keyPressEvent(QKeyEvent*)
{
    // This prevents the dialog to be closed if the ENTER key is pressed anywhere
}

#include "SettingsDialog.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:

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

#include "SettingsDialog.h"
#include "DatabaseBackendPage.h"
#include "ExifPage.h"
#include "PluginsPage.h"
#include "ViewerPage.h"
#include "FileVersionDetectionPage.h"
#include "ThumbnailsPage.h"
#include "GeneralPage.h"
#include "TagGroupsPage.h"
#include "CategoryPage.h"
#include "SettingsDialog.moc"
#include <QDebug>

#include <klocale.h>
#include <kglobal.h>
#include "Utilities/ShowBusyCursor.h"

#include "config-kpa-kipi.h"

#include "config-kpa-kface.h"
#ifdef HAVE_KFACE
#include "FaceManagementPage.h"
#endif

#include "BirthdayPage.h"

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
        { i18n("General"), "kphotoalbum", m_generalPage },
        { i18n("File Searching & Versions"), "system-search", m_fileVersionDetectionPage },
        { i18n("Thumbnail View" ), "view-list-icons", m_thumbnailsPage },
        { i18n("Categories"), "user-identity", m_categoryPage },
        { i18n("Birthdays"), "office-calendar", m_birthdayPage },
        { i18n("Tag Groups" ), "edit-copy", m_tagGroupsPage },
        { i18n("Viewer" ), "document-preview", m_viewerPage },
#ifdef HASKIPI
        { i18n("Plugins" ), "preferences-plugin", m_pluginsPage },
#endif

#ifdef HAVE_EXIV2
        { i18n("EXIF/IPTC Information" ), "document-properties", m_exifPage },
#endif
        { i18n("Database backend"), "system-file-manager", m_databaseBackendPage },
#ifdef HAVE_KFACE
        { i18n("Face management" ), "edit-find-user", m_faceManagementPage },
#endif
        { QString(), "", 0 }
    };

    int i = 0;
    while ( data[i].widget != 0 ) {
        KPageWidgetItem* page = new KPageWidgetItem( data[i].widget, data[i].title );
        page->setHeader( data[i].title );
        page->setIcon( KIcon( QString::fromLatin1( data[i].icon ) ) );
        addPage( page );
        ++i;
    }


    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    setCaption( i18n( "Settings" ) );

    connect(m_categoryPage, SIGNAL(currentCategoryNameChanged(QString,QString)),
            m_tagGroupsPage, SLOT(categoryRenamed(QString,QString)));
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
            m_tagGroupsPage, SLOT(slotPageChange()));
#ifdef HAVE_KFACE
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
            m_faceManagementPage, SLOT(slotPageChange(KPageWidgetItem*)));
#endif
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)),
            m_birthdayPage, SLOT(pageChange(KPageWidgetItem*)));
    connect(this, SIGNAL(cancelClicked()), m_birthdayPage, SLOT(discardChanges()));

    connect( this, SIGNAL(applyClicked()), this, SLOT(slotMyOK()) );
    connect( this, SIGNAL(okClicked()), this, SLOT(slotMyOK()) );
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

    KDialog::show();
}

// KDialog has a slotOK which we do not want to override.
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
    KGlobal::config()->sync();
}

void Settings::SettingsDialog::showBackendPage()
{
    setCurrentPage(m_backendPage);
}

void Settings::SettingsDialog::keyPressEvent(QKeyEvent*)
{
    // This prevents the dialog to be closed if the ENTER key is pressed anywhere
}

// vi:expandtab:tabstop=4 shiftwidth=4:

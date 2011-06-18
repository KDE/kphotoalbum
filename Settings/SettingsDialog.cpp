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
#include "SubCategoriesPage.h"
#include "CategoryPage.h"
#include "SettingsDialog.moc"

#include <klocale.h>
#include <kglobal.h>
#include "Utilities/ShowBusyCursor.h"

#include "config-kpa-sqldb.h"
#include "config-kpa-kipi.h"
struct Data
{
    QString title;
    const char* icon;
    QWidget* widget;
};

Settings::SettingsDialog::SettingsDialog( QWidget* parent)
     :KPageDialog( parent )
{
    _generalPage = new Settings::GeneralPage( this );
    _fileVersionDetectionPage = new Settings::FileVersionDetectionPage(this);
    _thumbnailsPage = new Settings::ThumbnailsPage( this );
    _categoryPage = new Settings::CategoryPage(this);
    _subCategoriesPage = new Settings::SubCategoriesPage( this );
    _viewerPage = new ViewerPage(this);

#ifdef HASKIPI
    _pluginsPage = new Settings::PluginsPage(this);
#endif

#ifdef HAVE_EXIV2
    _exifPage = new Settings::ExifPage(this);
#endif

    _databaseBackendPage = new Settings::DatabaseBackendPage(this);


    Data data[] = {
        { i18n("General"), "kphotoalbum", _generalPage },
        { i18n("File Searching & Versions"), "system-search", _fileVersionDetectionPage },
        { i18n("Thumbnail View" ), "view-list-icons", _thumbnailsPage },
        { i18n("Categories"), "user-identity", _categoryPage },
        { i18n("Subcategories" ), "edit-copy", _subCategoriesPage },
        { i18n("Viewer" ), "document-preview", _viewerPage },
#ifdef HASKIPI
        { i18n("Plugins" ), "preferences-plugin", _pluginsPage },
#endif

#ifdef HAVE_EXIV2
        { i18n("EXIF/IPTC Information" ), "document-properties", _exifPage },
#endif
        { i18n("Database backend"), "system-file-manager", _databaseBackendPage },
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

    connect( _categoryPage, SIGNAL( currentCategoryNameChanged( QString, QString ) ),
             _subCategoriesPage, SLOT( categoryRenamed( QString, QString ) ) );
    connect( this, SIGNAL( currentPageChanged(KPageWidgetItem*, KPageWidgetItem*) ), _subCategoriesPage, SLOT( slotPageChange() ) );
    connect( this, SIGNAL( applyClicked() ), this, SLOT( slotMyOK() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( slotMyOK() ) );
}

void Settings::SettingsDialog::show()
{
    Settings::SettingsData* opt = Settings::SettingsData::instance();

    _generalPage->loadSettings( opt );
    _fileVersionDetectionPage->loadSettings( opt );
    _thumbnailsPage->loadSettings(opt);
    _subCategoriesPage->loadSettings();
    _databaseBackendPage->loadSettings(opt);
    _viewerPage->loadSettings(opt);

#ifdef HASKIPI
    _pluginsPage->loadSettings(opt);
#endif

    _categoryPage->loadSettings(opt);

#ifdef HAVE_EXIV2
    _exifPage->loadSettings( opt );
#endif


    _categoryPage->enableDisable( false );

    KDialog::show();
}

// KDialog has a slotOK which we do not want to override.
void Settings::SettingsDialog::slotMyOK()
{
    Utilities::ShowBusyCursor dummy;
    Settings::SettingsData* opt = Settings::SettingsData::instance();

    // Must be before I save to the backend.
    if ( _thumbnailsPage->thumbnailSizeChanged(opt) )
        emit thumbnailSizeChanged();


    _generalPage->saveSettings( opt );
    _fileVersionDetectionPage->saveSettings( opt );
    _thumbnailsPage->saveSettings(opt);
    _categoryPage->saveSettings( opt, _subCategoriesPage->memberMap() );
    _subCategoriesPage->saveSettings();
    _viewerPage->saveSettings( opt );

#ifdef HASKIPI
    _pluginsPage->saveSettings( opt );
#endif

#ifdef HAVE_EXIV2
    _exifPage->saveSettings(opt);
#endif

    _databaseBackendPage->saveSettings(opt);

    emit changed();
    KGlobal::config()->sync();
}

void Settings::SettingsDialog::showBackendPage()
{
    setCurrentPage(_backendPage);
}


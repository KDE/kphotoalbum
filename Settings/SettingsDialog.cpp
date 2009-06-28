/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include "ThumbnailsPage.h"
#include "GeneralPage.h"
#include "SubCategoriesPage.h"
#include "CategoryPage.h"
#include "SettingsDialog.moc"

#include <kfiledialog.h>
#include <klocale.h>
#include <qlayout.h>
#include <qlabel.h>

#include <QHBoxLayout>
#include <QList>
#include <QGridLayout>
#include <QVBoxLayout>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <qspinbox.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include "Settings/SettingsData.h"
#include <kicondialog.h>
#include <q3listbox.h>
#include <kmessagebox.h>
#include "DB/ImageDB.h"
#include <qcheckbox.h>
#include <kinputdialog.h>
#include <QTextCodec>
#include <kglobal.h>
#include <kiconloader.h>
#include <q3vgroupbox.h>
#include <q3hbox.h>
#include "ViewerSizeConfig.h"
#include <limits.h>
#include "DB/CategoryCollection.h"
#include "Utilities/ShowBusyCursor.h"

#include <kapplication.h>
#include "MainWindow/Window.h"

#include "config-kpa-sqldb.h"
#ifdef SQLDB_SUPPORT
#  include "SQLDB/DatabaseAddress.h"
#  include "SQLDB/SQLSettingsWidget.h"
#endif

#include "CategoryItem.h"
#include <kdebug.h>

#include <QLineEdit>

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

#ifdef KDAB_TEMPORARILY_REMOVED
    Data data[] = {
        { i18n("General"), "kphotoalbum", _generalPage }
    };

    for ( int i = 0; i < 1; ++i ) {
        KPageWidgetItem* page = new KPageWidgetItem( data[i].widget, data[i].title );
        page->setHeader( data[i].title );
        page->setIcon( KIcon( QString::fromLatin1( data[i].icon ) ) );
        addPage( page );
    }
#endif //KDAB_TEMPORARILY_REMOVED


    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    setCaption( i18n( "Settings" ) );

    createGeneralPage();
    createThumbNailPage();
    createCategoryPage();
    createSubCategoriesPage();
    createViewerPage();

    createPluginPage();
    createEXIFPage();
    createDatabaseBackendPage();

    connect( _categoryPage, SIGNAL( currentCategoryNameChanged( QString, QString ) ),
             _subCategoriesPage, SLOT( categoryRenamed( QString, QString ) ) );
    connect( this, SIGNAL( currentPageChanged(KPageWidgetItem*, KPageWidgetItem*) ), _subCategoriesPage, SLOT( slotPageChange() ) );
    connect( this, SIGNAL( applyClicked() ), this, SLOT( slotMyOK() ) );
    connect( this, SIGNAL( okClicked() ), this, SLOT( slotMyOK() ) );
}

void Settings::SettingsDialog::createGeneralPage()
{
    _generalPage = new Settings::GeneralPage( this );

    KPageWidgetItem* page = new KPageWidgetItem( _generalPage, i18n("General" ) );
    page->setHeader( i18n("General") );
    page->setIcon( KIcon( QString::fromLatin1( "kphotoalbum" ) ) );
    addPage( page );
}

void Settings::SettingsDialog::createThumbNailPage()
{
    _thumbnailsPage = new Settings::ThumbnailsPage( this );
    KPageWidgetItem* page = new KPageWidgetItem( _thumbnailsPage, i18n("Thumbnail View" ) );
    page->setHeader( i18n("Thumbnail View" ) );
    page->setIcon( KIcon( QString::fromLatin1( "view-list-icons" ) ) );
    addPage( page );
}


void Settings::SettingsDialog::createCategoryPage()
{
    _categoryPage = new Settings::CategoryPage(this);
    KPageWidgetItem* page = new KPageWidgetItem( _categoryPage, i18n("Categories") );
    page->setHeader( i18n("Categories") );
    page->setIcon( KIcon( QString::fromLatin1( "user-identity" ) ) );
    addPage( page );
}


void Settings::SettingsDialog::show()
{
    Settings::SettingsData* opt = Settings::SettingsData::instance();

    _generalPage->loadSettings( opt );
    _databaseBackendPage->loadSettings(opt);
    _viewerPage->reset(opt);


#ifdef HASKIPI
    _pluginsPage->loadSettings(opt);
#endif

    _categoryPage->loadSettings();

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
    _generalPage->saveSettings( opt );
    _databaseBackendPage->saveSettings(opt);
    _categoryPage->saveSettings( _subCategoriesPage->memberMap() );
    _subCategoriesPage->saveSettings();

#ifdef HASKIPI
    _pluginsPage->saveSettings( opt );
#endif

    // EXIF
#ifdef HAVE_EXIV2
    _exifPage->saveSettings(opt);
#endif

    emit changed();
    KGlobal::config()->sync();
}




void Settings::SettingsDialog::createSubCategoriesPage()
{
    _subCategoriesPage = new Settings::SubCategoriesPage( this );
    KPageWidgetItem* page = new KPageWidgetItem( _subCategoriesPage, i18n("Subcategories" ) );
    page->setHeader( i18n("Subcategories" ) );
    page->setIcon( KIcon( QString::fromLatin1( "edit-copy" ) ) );
    addPage( page );
}




int Settings::SettingsDialog::exec()
{
    _subCategoriesPage->reset();
    return KDialog::exec();
}



void Settings::SettingsDialog::createViewerPage()
{
    _viewerPage = new ViewerPage(this);
    KPageWidgetItem* page = new KPageWidgetItem( _viewerPage, i18n("Viewer" ) );
    page->setHeader( i18n("Viewer" ) );
    page->setIcon( KIcon( QString::fromLatin1( "document-preview" ) ) );
    addPage( page );
}


void Settings::SettingsDialog::createPluginPage()
{
#ifdef HASKIPI
    _pluginsPage = new Settings::PluginsPage(this);
    KPageWidgetItem* page = new KPageWidgetItem( _pluginsPage, i18n("Plugins" ) );
    page->setHeader( i18n("Plugins" ) );
    page->setIcon( KIcon( QString::fromLatin1( "preferences-plugin" ) ) );
    addPage(page);

#endif
}

void Settings::SettingsDialog::createEXIFPage()
{
#ifdef HAVE_EXIV2
    _exifPage = new Settings::ExifPage(this);
    KPageWidgetItem* page = new KPageWidgetItem( _exifPage, i18n("EXIF/IPTC Information" ) );
    page->setHeader( i18n("EXIF Information" ) );
    page->setIcon( KIcon( QString::fromLatin1( "document-properties" ) ) );
    addPage( page );

#endif
}

void Settings::SettingsDialog::showBackendPage()
{
    setCurrentPage(_backendPage);
}

void Settings::SettingsDialog::createDatabaseBackendPage()
{
// TODO: add notification: New backend will take effect only after restart
    _databaseBackendPage = new Settings::DatabaseBackendPage(this);
    _backendPage = new KPageWidgetItem( _databaseBackendPage, i18n("Database backend") );
    _backendPage->setHeader( i18n("Database backend") );
    _backendPage->setIcon( KIcon( QString::fromLatin1("system-file-manager") ) );
    addPage( _backendPage );
}

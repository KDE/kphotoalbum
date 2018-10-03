/* Copyright (C) 2003-2016 Jesper K. Pedersen <blackie@kde.org>

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

#include "OverviewPage.h"

#include "BrowserWidget.h"
#include "CategoryPage.h"
#include "enums.h"
#include "ImageViewPage.h"

#include <AnnotationDialog/Dialog.h>
#ifdef HAVE_KGEOMAP
#include <Browser/GeoPositionPage.h>
#endif
#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <Exif/SearchDialog.h>
#include <MainWindow/Logging.h>
#include <Utilities/ShowBusyCursor.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <MainWindow/Window.h>

#include <QElapsedTimer>
#include <QIcon>
#include <QPixmap>

const int THUMBNAILSIZE = 70;

AnnotationDialog::Dialog* Browser::OverviewPage::s_config = nullptr;
Browser::OverviewPage::OverviewPage( const Breadcrumb& breadcrumb, const DB::ImageSearchInfo& info, BrowserWidget* browser )
    : BrowserPage( info, browser), m_breadcrumb( breadcrumb )
{
    //updateImageCount();
}

int Browser::OverviewPage::rowCount( const QModelIndex& parent ) const
{
    if ( parent != QModelIndex() )
        return 0;

    return categories().count() +
#ifdef HAVE_KGEOMAP
        1 +
#endif
        4; // Exiv search + Search info + Untagged Images + Show Image
}

QVariant Browser::OverviewPage::data( const QModelIndex& index, int role) const
{
    if ( role == ValueRole )
        return index.row();

    const int row = index.row();
    if ( isCategoryIndex(row) )
        return categoryInfo( row, role );
#ifdef HAVE_KGEOMAP
    else if ( isGeoPositionIndex( row ) )
        return geoPositionInfo( role );
#endif
    else if ( isExivIndex( row ) )
        return exivInfo( role );
    else if ( isSearchIndex( row ) )
        return searchInfo( role );
    else if ( isUntaggedImagesIndex( row ) )
        return untaggedImagesInfo( role );
    else if ( isImageIndex( row ) )
        return imageInfo( role );
    return QVariant();
}

bool Browser::OverviewPage::isCategoryIndex( int row ) const
{
    return row < categories().count() && row >= 0;
}

bool Browser::OverviewPage::isGeoPositionIndex( int row ) const
{
#ifdef HAVE_KGEOMAP
    return row == categories().count();
#else
    Q_UNUSED(row);
    return false;
#endif
}

bool Browser::OverviewPage::isExivIndex( int row ) const
{
    int exivRow = categories().count();
    #ifdef HAVE_KGEOMAP
        exivRow++;
    #endif
    return row == exivRow;
}

bool Browser::OverviewPage::isSearchIndex( int row ) const
{
    return rowCount()-3 == row;
}

bool Browser::OverviewPage::isUntaggedImagesIndex( int row ) const
{
    return rowCount()-2 == row;

}

bool Browser::OverviewPage::isImageIndex( int row ) const
{
    return rowCount()-1 == row;
}


QList<DB::CategoryPtr> Browser::OverviewPage::categories() const
{
    return DB::ImageDB::instance()->categoryCollection()->categories();
}

QVariant Browser::OverviewPage::categoryInfo( int row, int role ) const
{
    if ( role == Qt::DisplayRole )
        return categories()[row]->name();
    else if ( role == Qt::DecorationRole )
        return categories()[row]->icon(THUMBNAILSIZE);

    return QVariant();
}

QVariant Browser::OverviewPage::geoPositionInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Geo Position");
    else if ( role == Qt::DecorationRole ) {
        return QIcon::fromTheme(QString::fromLatin1("globe")).pixmap(THUMBNAILSIZE);
    }

    return QVariant();
}

QVariant Browser::OverviewPage::exivInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Exif Info");
    else if ( role == Qt::DecorationRole ) {
        return QIcon::fromTheme(QString::fromLatin1("document-properties")).pixmap(THUMBNAILSIZE);
    }

    return QVariant();
}

QVariant Browser::OverviewPage::searchInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18nc("@action Search button in the browser view.","Search");
    else if ( role == Qt::DecorationRole )
        return QIcon::fromTheme( QString::fromLatin1( "system-search" ) ).pixmap(THUMBNAILSIZE);
    return QVariant();
}

QVariant Browser::OverviewPage::untaggedImagesInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Untagged Images");
    else if ( role == Qt::DecorationRole )
        return QIcon::fromTheme(QString::fromUtf8("archive-insert")).pixmap(THUMBNAILSIZE);
    return QVariant();

}

QVariant Browser::OverviewPage::imageInfo( int role ) const
{
    if ( role == Qt::DisplayRole )
        return i18n("Show Thumbnails");
    else if ( role == Qt::DecorationRole )
    {
        QIcon icon = QIcon::fromTheme(QString::fromUtf8("view-preview"));
        QPixmap pixmap = icon.pixmap(THUMBNAILSIZE);
        // workaround for QListView in Qt 5.5:
        // On Qt5.5 if the last item in the list view has no DecorationRole, then
        // the whole list view "collapses" to the size of text-only items,
        // cutting off the existing thumbnails.
        // This can be triggered by an incomplete icon theme.
        if (pixmap.isNull())
        {
            pixmap = QPixmap(THUMBNAILSIZE,THUMBNAILSIZE);
            pixmap.fill(Qt::transparent);
        }
        return pixmap;
    }
    return QVariant();
}

Browser::BrowserPage* Browser::OverviewPage::activateChild( const QModelIndex& index )
{
    const int row = index.row();

    if ( isCategoryIndex(row) )
        return new Browser::CategoryPage( categories()[row], BrowserPage::searchInfo(), browser() );
#ifdef HAVE_KGEOMAP
    else if ( isGeoPositionIndex( row ) )
        return new Browser::GeoPositionPage( BrowserPage::searchInfo(), browser() );
#endif
    else if ( isExivIndex( row ) )
        return activateExivAction();
    else if ( isSearchIndex( row ) )
        return activateSearchAction();
    else if ( isUntaggedImagesIndex( row ) ) {
        return activateUntaggedImagesAction();
    }
    else if ( isImageIndex( row ) )
        return new ImageViewPage( BrowserPage::searchInfo(), browser()  );

    return nullptr;
}

void Browser::OverviewPage::activate()
{
    updateImageCount();
    browser()->setModel( this );
}

Qt::ItemFlags Browser::OverviewPage::flags( const QModelIndex & index ) const
{
    if ( isCategoryIndex(index.row() ) && m_count[index.row()] <= 1 )
        return QAbstractListModel::flags(index) & ~Qt::ItemIsEnabled;
    else
        return QAbstractListModel::flags(index);
}

bool Browser::OverviewPage::isSearchable() const
{
    return true;
}

Browser::BrowserPage* Browser::OverviewPage::activateExivAction()
{
    QPointer<Exif::SearchDialog> dialog = new Exif::SearchDialog( browser() );

    {
        Utilities::ShowBusyCursor undoTheBusyWhileShowingTheDialog( Qt::ArrowCursor );

        if ( dialog->exec() == QDialog::Rejected ) {
            delete dialog;
            return nullptr;
        }
        // Dialog can be deleted by its parent in event loop while in exec()
        if ( dialog.isNull() )
            return nullptr;
    }


    Exif::SearchInfo result = dialog->info();

    DB::ImageSearchInfo info = BrowserPage::searchInfo();

    info.addExifSearchInfo( dialog->info() );

    delete dialog;

    if ( DB::ImageDB::instance()->count( info ).total() == 0 ) {
        KMessageBox::information( browser(), i18n( "Search did not match any images or videos." ), i18n("Empty Search Result") );
        return nullptr;
    }

    return new OverviewPage( Breadcrumb( i18n("Exif Search")), info, browser() );
}

Browser::BrowserPage* Browser::OverviewPage::activateSearchAction()
{
    if ( !s_config )
        s_config = new AnnotationDialog::Dialog( browser() );

    Utilities::ShowBusyCursor undoTheBusyWhileShowingTheDialog( Qt::ArrowCursor );
    DB::ImageSearchInfo tmpInfo = BrowserPage::searchInfo();
    DB::ImageSearchInfo info = s_config->search( &tmpInfo ); // PENDING(blackie) why take the address?

    if ( info.isNull() )
        return nullptr;

    if ( DB::ImageDB::instance()->count( info ).total() == 0 ) {
        KMessageBox::information( browser(), i18n( "Search did not match any images or videos." ), i18n("Empty Search Result") );
        return nullptr;
    }

    return new OverviewPage( Breadcrumb( i18nc("Breadcrumb denoting that we 'browsed' to a search result.","search") ), info, browser() );

}

Browser::Breadcrumb Browser::OverviewPage::breadcrumb() const
{
    return m_breadcrumb;
}

bool Browser::OverviewPage::showDuringMovement() const
{
    return true;
}

void Browser::OverviewPage::updateImageCount()
{
    QElapsedTimer timer;
    timer.start();
    int row = 0;
    for (const DB::CategoryPtr& category : categories() ) {
        QMap<QString, uint> items = DB::ImageDB::instance()->classify( BrowserPage::searchInfo(), category->name(), DB::anyMediaType );
        m_count[row] = items.count();
        ++row;
    }
    qCDebug(TimingLog) << "Browser::Overview::updateImageCount(): " << timer.elapsed() << "ms.";
}

Browser::BrowserPage* Browser::OverviewPage::activateUntaggedImagesAction()
{
    if ( Settings::SettingsData::instance()->hasUntaggedCategoryFeatureConfigured() ) {
        DB::ImageSearchInfo info = BrowserPage::searchInfo();
        info.addAnd( Settings::SettingsData::instance()->untaggedCategory(),
                                   Settings::SettingsData::instance()->untaggedTag() );
        return new ImageViewPage( info, browser()  );
    }
    else {
        // Note: the same dialog text is used in MainWindow::Window::slotMarkUntagged(),
        // so if it is changed, be sure to also change it there!
        KMessageBox::information( browser(),
                                  i18n("<p>You have not yet configured which tag to use for indicating untagged images.</p>"
                                       "<p>Please follow these steps to do so:"
                                       "<ul><li>In the menu bar choose <b>Settings</b></li>"
                                       "<li>From there choose <b>Configure KPhotoAlbum</b></li>"
                                       "<li>Now choose the <b>Categories</b> icon</li>"
                                       "<li>Now configure section <b>Untagged Images</b></li></ul></p>"),
                                  i18n("Feature has not been configured") );
        return nullptr;
    }
}



// vi:expandtab:tabstop=4 shiftwidth=4:

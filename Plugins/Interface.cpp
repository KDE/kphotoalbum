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

#include "Interface.h"
#include <QList>
#include <klocale.h>
#include <kimageio.h>
#include <KIO/PreviewJob>
#include <libkipi/imagecollection.h>
#include "Browser/BrowserWidget.h"
#include "Browser/TreeCategoryModel.h"
#include "ImageManager/RawImageDecoder.h"
#include "ImageManager/ThumbnailCache.h"
#include "MainWindow/Window.h"
#include "Plugins/CategoryImageCollection.h"
#include "Plugins/ImageCollection.h"
#include "Plugins/ImageCollectionSelector.h"
#include "Plugins/ImageInfo.h"
#include "DB/ImageDB.h"
#include "DB/ImageInfo.h"
#include "DB/CategoryCollection.h"
#include "UploadWidget.h"
#include "Utilities/Util.h"
namespace KIPI { class UploadWidget; }

Plugins::Interface::Interface( QObject *parent, const char *name )
    :KIPI::Interface( parent, name )
{
    connect( Browser::BrowserWidget::instance(), SIGNAL(pathChanged(Browser::BreadcrumbList)), this, SLOT(pathChanged(Browser::BreadcrumbList)) );
}

KIPI::ImageCollection Plugins::Interface::currentAlbum()
{
    return KIPI::ImageCollection( new Plugins::ImageCollection( Plugins::ImageCollection::CurrentAlbum ) );
}

KIPI::ImageCollection Plugins::Interface::currentSelection()
{
    if (!MainWindow::Window::theMainWindow()->selected().isEmpty())
        return KIPI::ImageCollection( new Plugins::ImageCollection( Plugins::ImageCollection::CurrentSelection ) );
    else
        return KIPI::ImageCollection(nullptr);
}

QList<KIPI::ImageCollection> Plugins::Interface::allAlbums()
{
    QList<KIPI::ImageCollection> result;
    DB::ImageSearchInfo context = MainWindow::Window::theMainWindow()->currentContext();
    QString category = MainWindow::Window::theMainWindow()->currentBrowseCategory();
    if ( category.isNull() )
        category = Settings::SettingsData::instance()->albumCategory();

    QMap<QString,uint> categories = DB::ImageDB::instance()->classify( context, category, DB::Image );

    for( QMap<QString,uint>::iterator it = categories.begin(); it != categories.end(); ++it ) {
        CategoryImageCollection* col = new CategoryImageCollection( context, category, it.key() );
        result.append( KIPI::ImageCollection( col ) );
    }

    return result;
}

KIPI::ImageInfo Plugins::Interface::info( const KUrl& url )
{
    return KIPI::ImageInfo(new Plugins::ImageInfo(this, url));
}

void Plugins::Interface::refreshImages( const KUrl::List& urls )
{
    emit imagesChanged( urls );
}

int Plugins::Interface::features() const
{
    return
        KIPI::ImagesHasComments |
        KIPI::ImagesHasTime |
        KIPI::HostSupportsDateRanges |
        KIPI::HostAcceptNewImages |
        KIPI::ImagesHasTitlesWritable |
        KIPI::HostSupportsTags |
        KIPI::HostSupportsThumbnails |
        KIPI::HostSupportsRating;
}

QAbstractItemModel * Plugins::Interface::getTagTree() const
{
    DB::ImageSearchInfo matchAll;
    DB::CategoryPtr rootCategory;

    // since this is currently used by the geolocation plugin only, try the (localized) "Places" category first:
    rootCategory = DB::ImageDB::instance()->categoryCollection()->categoryForName( i18n( "Places" ));

    // ... if that's not available, return a category that exists:
    if ( rootCategory.isNull() )
        rootCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial( DB::Category::TokensCategory );

    return new Browser::TreeCategoryModel( rootCategory , matchAll );
}

QVariant Plugins::Interface::hostSetting( const QString& settingName )
{
    if (settingName == QString::fromUtf8("WriteMetadataUpdateFiletimeStamp"))
        return false;
    if (settingName == QString::fromUtf8("WriteMetadataToRAW"))
        return false;

    if (settingName == QString::fromUtf8("UseXMPSidecar4Reading"))
        return false;
    if (settingName == QString::fromUtf8("MetadataWritingMode"))
        return 0; /* WRITETOIMAGEONLY */

    bool fileExt = settingName == QString::fromUtf8("FileExtensions");
    bool imageExt = fileExt || settingName == QString::fromUtf8("ImagesExtensions");
    bool rawExt   = fileExt || settingName == QString::fromUtf8("RawExtensions");
    bool videoExt = fileExt || settingName == QString::fromUtf8("VideoExtensions");
    if ( imageExt || rawExt || videoExt )
    {
        QStringList fileTypes;
        if ( imageExt )
            // Return a list of images file extensions supported by KDE.
            // This works as long as Settings::SettingsData::instance()->ignoreFileExtension() is not true
            fileTypes += KImageIO::mimeTypes( KImageIO::Reading );

        if ( rawExt )
            fileTypes += ImageManager::RAWImageDecoder::rawExtensions();

        if ( videoExt )
            fileTypes += Utilities::supportedVideoExtensions().toList();

        QString fileFilter = fileTypes.join(QString::fromUtf8(" "));
        return QString( fileFilter.toLower() + QString::fromUtf8(" ") + fileFilter.toUpper() );
    }

    if ( settingName == QString::fromUtf8("AudioExtensions") )
        return QString();

    return QVariant();
}

bool Plugins::Interface::addImage( const KUrl& url, QString& errmsg )
{
    const QString dir = url.path();
    const QString root = Settings::SettingsData::instance()->imageDirectory();
    if ( !dir.startsWith( root ) ) {
        errmsg = i18n("<p>Image needs to be placed in a sub directory of your photo album, "
                      "which is rooted at %1. Image path was %2</p>",root , dir );
        return false;
    }

    DB::ImageInfoPtr info( new DB::ImageInfo( DB::FileName::fromAbsolutePath(dir) ) );
    DB::ImageInfoList list;
    list.append( info );
    DB::ImageDB::instance()->addImages( list );
    return true;
}

void Plugins::Interface::delImage( const KUrl& url )
{
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( DB::FileName::fromAbsolutePath(url.path()));
    if ( info )
        DB::ImageDB::instance()->deleteList(DB::FileNameList() << info->fileName() );
}

void Plugins::Interface::slotSelectionChanged( bool b )
{
    emit selectionChanged( b );
}

void Plugins::Interface::pathChanged( const Browser::BreadcrumbList& path )
{
    static Browser::BreadcrumbList _path;
    if ( _path != path ) {
        emit currentAlbumChanged( true );
        _path = path;
    }
}

KIPI::ImageCollectionSelector* Plugins::Interface::imageCollectionSelector(QWidget *parent)
{
    return new ImageCollectionSelector( parent, this );
}

KIPI::UploadWidget* Plugins::Interface::uploadWidget(QWidget* parent)
{
    return new Plugins::UploadWidget(parent);
}

void Plugins::Interface::thumbnail(const KUrl &url, int size)
{
    DB::FileName file = DB::FileName::fromAbsolutePath( url.path() );
    if (size <= Settings::SettingsData::instance()->thumbnailSize()
            && ImageManager::ThumbnailCache::instance()->contains(file))
    {
        // look up in the cache
        QPixmap thumb = ImageManager::ThumbnailCache::instance()->lookup( file );
        emit gotThumbnail( url, thumb);
    } else {
        // for bigger thumbnails, fall back to previewJob:
        KFileItem f = KFileItem(KFileItem::Unknown, KFileItem::Unknown, url, true);
        KFileItemList fl;
        fl.append(f);
        KIO::PreviewJob *job = KIO::filePreview( fl, QSize(size,size));

        connect(job, SIGNAL(gotPreview(KFileItem,QPixmap)),
                this, SLOT(gotKDEPreview(KFileItem,QPixmap)));

        connect(job, SIGNAL(failed(KFileItem)),
                this, SLOT(failedKDEPreview(KFileItem)));
    }
}

void Plugins::Interface::thumbnails(const KUrl::List &list, int size)
{
    for (const KUrl url : list)
       thumbnail( url, size );
}

void Plugins::Interface::gotKDEPreview(const KFileItem& item, const QPixmap& pix)
{
    emit gotThumbnail(item.url(), pix);
}

void Plugins::Interface::failedKDEPreview(const KFileItem& item)
{
    emit gotThumbnail(item.url(), QPixmap());
}

#include "Interface.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:

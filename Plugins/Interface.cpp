/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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

#include <QByteArray>
#include <QImageReader>
#include <QList>

#include <KFileItem>
#include <KIO/PreviewJob>
#include <KIPI/ImageCollection>
#include <KLocalizedString>

#include "Utilities/FileUtil.h"
#include <Browser/BrowserWidget.h>
#include <Browser/TreeCategoryModel.h>
#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <ImageManager/ThumbnailCache.h>
#include <MainWindow/Window.h>
#include <Plugins/CategoryImageCollection.h>
#include <Plugins/ImageCollection.h>
#include <Plugins/ImageCollectionSelector.h>
#include <Plugins/ImageInfo.h>

#include "UploadWidget.h"
namespace KIPI
{
class UploadWidget;
}

Plugins::Interface::Interface(QObject *parent, QString name)
    : KIPI::Interface(parent, name)
{
    connect(Browser::BrowserWidget::instance(), SIGNAL(pathChanged(Browser::BreadcrumbList)), this, SLOT(pathChanged(Browser::BreadcrumbList)));
}

KIPI::ImageCollection Plugins::Interface::currentAlbum()
{
    return KIPI::ImageCollection(new Plugins::ImageCollection(Plugins::ImageCollection::CurrentAlbum));
}

KIPI::ImageCollection Plugins::Interface::currentSelection()
{
    if (!MainWindow::Window::theMainWindow()->selected().isEmpty())
        return KIPI::ImageCollection(new Plugins::ImageCollection(Plugins::ImageCollection::CurrentSelection));
    else
        return KIPI::ImageCollection(nullptr);
}

QList<KIPI::ImageCollection> Plugins::Interface::allAlbums()
{
    QList<KIPI::ImageCollection> result;
    DB::ImageSearchInfo context = MainWindow::Window::theMainWindow()->currentContext();
    QString category = MainWindow::Window::theMainWindow()->currentBrowseCategory();
    if (category.isNull())
        category = Settings::SettingsData::instance()->albumCategory();

    QMap<QString, DB::CountWithRange> categories = DB::ImageDB::instance()->classify(context, category, DB::Image);

    for (auto it = categories.constBegin(); it != categories.constEnd(); ++it) {
        auto *col = new CategoryImageCollection(context, category, it.key());
        result.append(KIPI::ImageCollection(col));
    }

    return result;
}

KIPI::ImageInfo Plugins::Interface::info(const QUrl &url)
{
    return KIPI::ImageInfo(new Plugins::ImageInfo(this, url));
}

void Plugins::Interface::refreshImages(const QList<QUrl> &urls)
{
    emit imagesChanged(urls);
}

int Plugins::Interface::features() const
{
    return KIPI::ImagesHasComments | KIPI::ImagesHasTime | KIPI::HostSupportsDateRanges | KIPI::HostAcceptNewImages | KIPI::ImagesHasTitlesWritable | KIPI::HostSupportsTags | KIPI::HostSupportsRating | KIPI::HostSupportsThumbnails;
}

QAbstractItemModel *Plugins::Interface::getTagTree() const
{
    DB::ImageSearchInfo matchAll;
    DB::CategoryPtr rootCategory;

    // since this is currently used by the geolocation plugin only, try the (localized) "Places" category first:
    rootCategory = DB::ImageDB::instance()->categoryCollection()->categoryForName(i18n("Places"));

    // ... if that's not available, return a category that exists:
    if (!rootCategory)
        rootCategory = DB::ImageDB::instance()->categoryCollection()->categoryForSpecial(DB::Category::TokensCategory);

    return new Browser::TreeCategoryModel(rootCategory, matchAll);
}

bool Plugins::Interface::addImage(const QUrl &url, QString &errmsg)
{
    const QString dir = url.path();
    const QString root = Settings::SettingsData::instance()->imageDirectory();
    if (!dir.startsWith(root)) {
        errmsg = i18n("<p>Image needs to be placed in a sub directory of your photo album, "
                      "which is rooted at %1. Image path was %2</p>",
                      root, dir);
        return false;
    }

    DB::ImageInfoPtr info(new DB::ImageInfo(DB::FileName::fromAbsolutePath(dir)));
    DB::ImageInfoList list;
    list.append(info);
    DB::ImageDB::instance()->addImages(list);
    return true;
}

void Plugins::Interface::delImage(const QUrl &url)
{
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info(DB::FileName::fromAbsolutePath(url.path()));
    if (info)
        DB::ImageDB::instance()->deleteList(DB::FileNameList() << info->fileName());
}

void Plugins::Interface::slotSelectionChanged(bool b)
{
    emit selectionChanged(b);
}

void Plugins::Interface::pathChanged(const Browser::BreadcrumbList &path)
{
    static Browser::BreadcrumbList _path;
    if (_path != path) {
        emit currentAlbumChanged(true);
        _path = path;
    }
}

KIPI::ImageCollectionSelector *Plugins::Interface::imageCollectionSelector(QWidget *parent)
{
    return new ImageCollectionSelector(parent, this);
}

KIPI::UploadWidget *Plugins::Interface::uploadWidget(QWidget *parent)
{
    return new Plugins::UploadWidget(parent);
}

void Plugins::Interface::thumbnail(const QUrl &url, int size)
{
    DB::FileName file = DB::FileName::fromAbsolutePath(url.path());
    if (size <= Settings::SettingsData::instance()->thumbnailSize()
        && ImageManager::ThumbnailCache::instance()->contains(file)) {
        // look up in the cache
        QPixmap thumb = ImageManager::ThumbnailCache::instance()->lookup(file);
        emit gotThumbnail(url, thumb);
    } else {
        // for bigger thumbnails, fall back to previewJob:
        KFileItem f { url };
        f.setDelayedMimeTypes(true);
        KFileItemList fl;
        fl.append(f);
        KIO::PreviewJob *job = KIO::filePreview(fl, QSize(size, size));

        connect(job, &KIO::PreviewJob::gotPreview, this, &Interface::gotKDEPreview);

        connect(job, &KIO::PreviewJob::failed, this, &Interface::failedKDEPreview);
    }
}

void Plugins::Interface::thumbnails(const QList<QUrl> &list, int size)
{
    for (const QUrl url : list)
        thumbnail(url, size);
}

KIPI::FileReadWriteLock *Plugins::Interface::createReadWriteLock(const QUrl &) const
{
    return nullptr;
}

KIPI::MetadataProcessor *Plugins::Interface::createMetadataProcessor() const
{
    return nullptr;
}

void Plugins::Interface::gotKDEPreview(const KFileItem &item, const QPixmap &pix)
{
    emit gotThumbnail(item.url(), pix);
}

void Plugins::Interface::failedKDEPreview(const KFileItem &item)
{
    emit gotThumbnail(item.url(), QPixmap());
}

// vi:expandtab:tabstop=4 shiftwidth=4:

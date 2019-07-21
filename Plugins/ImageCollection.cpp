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

#include "ImageCollection.h"

#include "Logging.h"

#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <DB/ImageInfoList.h>
#include <MainWindow/Window.h>
#include <Settings/SettingsData.h>

#include <KLocalizedString>
#include <QFileInfo>

Plugins::ImageCollection::ImageCollection(Type tp)
    : m_type(tp)
{
}

QString Plugins::ImageCollection::name()
{
    QString res;
    switch (m_type) {
    case CurrentAlbum:
        res = MainWindow::Window::theMainWindow()->currentContext().toString();
        break;
    case CurrentSelection:
        res = MainWindow::Window::theMainWindow()->currentContext().toString();
        if (res.isEmpty()) {
            res = i18nc("As in 'an unknown set of images, created from the selection'.", "Unknown (Selection)");
        } else {
            res += i18nc("As in 'A selection of [a generated context description]'", " (Selection)");
        }
        break;
    case SubClass:
        qCWarning(PluginsLog, "Subclass of ImageCollection should overwrite ImageCollection::name()");
        res = i18nc("A set of images with no description.", "Unknown");
        break;
    }
    if (res.isEmpty()) {
        // at least html export plugin needs a non-empty name:
        res = i18nc("The 'name' of an unnamed image collection.", "None");
    }
    return res;
}

QList<QUrl> Plugins::ImageCollection::images()
{
    switch (m_type) {
    case CurrentAlbum:
        return stringListToUrlList(DB::ImageDB::instance()->currentScope(false).toStringList(DB::AbsolutePath));

    case CurrentSelection:
        return stringListToUrlList(MainWindow::Window::theMainWindow()->selected(ThumbnailView::NoExpandCollapsedStacks).toStringList(DB::AbsolutePath));

    case SubClass:
        qFatal("The subclass should implement images()");
        return QList<QUrl>();
    }
    return QList<QUrl>();
}

QList<QUrl> Plugins::ImageCollection::imageListToUrlList(const DB::ImageInfoList &imageList)
{
    QList<QUrl> urlList;
    for (DB::ImageInfoListConstIterator it = imageList.constBegin(); it != imageList.constEnd(); ++it) {
        QUrl url;
        url.setPath((*it)->fileName().absolute());
        urlList.append(url);
    }
    return urlList;
}

QList<QUrl> Plugins::ImageCollection::stringListToUrlList(const QStringList &list)
{
    QList<QUrl> urlList;
    for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it) {
        QUrl url;
        url.setPath(*it);
        urlList.append(url);
    }
    return urlList;
}

QUrl Plugins::ImageCollection::url()
{
    return commonRoot();
}

QUrl Plugins::ImageCollection::commonRoot()
{
    QString imgRoot = Settings::SettingsData::instance()->imageDirectory();
    const QList<QUrl> imgs = images();
    if (imgs.count() == 0)
        return QUrl::fromLocalFile(imgRoot);

    QStringList res = QFileInfo(imgs[0].path()).absolutePath().split(QLatin1String("/"));

    for (QList<QUrl>::ConstIterator it = imgs.begin(); it != imgs.end(); ++it) {
        QStringList newRes;

        QStringList path = QFileInfo((*it).path()).absolutePath().split(QLatin1String("/"));
        int i = 0;
        for (; i < qMin(path.size(), res.size()); ++i) {
            if (path[i] == res[i])
                newRes.append(res[i]);
            else
                break;
        }
        res = newRes;
    }

    QString result = res.join(QString::fromLatin1("/"));
    if (result.left(imgRoot.length()) != imgRoot) {
        result = imgRoot;
    }

    return QUrl::fromLocalFile(result);
}

QUrl Plugins::ImageCollection::uploadUrl()
{
    return commonRoot();
}

QUrl Plugins::ImageCollection::uploadRootUrl()
{
    QUrl url = QUrl::fromLocalFile(Settings::SettingsData::instance()->imageDirectory());
    return url;
}

QString Plugins::ImageCollection::uploadRootName()
{
    return i18nc("'Name' of the image directory", "Image/Video root directory");
}

// vi:expandtab:tabstop=4 shiftwidth=4:

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

#ifndef KPHOTOALBUM_PLUGININTERFACE_H
#define KPHOTOALBUM_PLUGININTERFACE_H

#include <KIPI/ImageCollection>
#include <KIPI/ImageCollectionSelector>
#include <KIPI/ImageInfo>
#include <KIPI/Interface>
#include <QList>
#include <QUrl>
#include <QVariant>
#include <config-kpa-kipi.h>

class QPixmap;
class KFileItem;

namespace Browser
{
class BreadcrumbList;
}

namespace Plugins
{

class Interface : public KIPI::Interface
{
    Q_OBJECT

public:
    explicit Interface(QObject *parent, QString name = QString());

    KIPI::ImageCollection currentAlbum() override;
    KIPI::ImageCollection currentSelection() override;
    QList<KIPI::ImageCollection> allAlbums() override;

    KIPI::ImageInfo info(const QUrl &) override;
    bool addImage(const QUrl &, QString &errmsg) override;
    void delImage(const QUrl &) override;
    void refreshImages(const QList<QUrl> &urls) override;

    void thumbnail(const QUrl &url, int size) override;
    void thumbnails(const QList<QUrl> &list, int size) override;

    KIPI::ImageCollectionSelector *imageCollectionSelector(QWidget *parent) override;
    KIPI::UploadWidget *uploadWidget(QWidget *parent) override;
    QAbstractItemModel *getTagTree() const override;

    // these two methods are only here because of a libkipi api error
    // either remove them when they are no longer pure virtual in KIPI::Interface,
    // or implement them and update features() accordingly:
    // FIXME: this can be safely removed if/when libkipi 5.1.0 is no longer supported
    KIPI::FileReadWriteLock *createReadWriteLock(const QUrl &) const override;
    KIPI::MetadataProcessor *createMetadataProcessor() const override;

    int features() const override;

public slots:
    void slotSelectionChanged(bool);
    void pathChanged(const Browser::BreadcrumbList &path);

private slots:
    void gotKDEPreview(const KFileItem &item, const QPixmap &pix);
    void failedKDEPreview(const KFileItem &item);

signals:
    void imagesChanged(const QList<QUrl> &);
};

}

#endif /* PLUGININTERFACE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

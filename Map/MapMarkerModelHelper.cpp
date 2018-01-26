/* Copyright (C) 2014-2018 Johannes Zarl-Zierl <johannes@zarl-zierl.at>

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

#include "MapMarkerModelHelper.h"

#include "Logging.h"

// Qt includes
#include <QItemSelectionModel>
#include <QStandardItem>
#include <QStandardItemModel>

// Local includes
#include <ImageManager/ThumbnailCache.h>

// libkgeomap includes
#include <KGeoMap/GeoCoordinates>

const int FileNameRole = Qt::UserRole + 1;

Map::MapMarkerModelHelper::MapMarkerModelHelper()
    : m_itemModel(0)
    , m_itemSelectionModel(0)
{
    m_itemModel = new QStandardItemModel(1, 1);
    m_itemSelectionModel = new QItemSelectionModel(m_itemModel);
    connect(m_itemModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(slotDataChanged(QModelIndex, QModelIndex)));
}

Map::MapMarkerModelHelper::~MapMarkerModelHelper()
{
    delete m_itemSelectionModel;
    delete m_itemModel;
}

void Map::MapMarkerModelHelper::clearItems()
{
    m_itemModel->clear();
}

void Map::MapMarkerModelHelper::addImage(const DB::ImageInfo &image)
{
    qCDebug(MapLog) << "Adding marker for image " << image.label();
    QStandardItem *const newItem = new QStandardItem(image.label());


    newItem->setToolTip( image.label() );
    newItem->setData( QVariant::fromValue( image.fileName() ), FileNameRole );
    m_itemModel->appendRow( newItem );
}

void Map::MapMarkerModelHelper::addImage( const DB::ImageInfoPtr image )
{
    addImage( *image );
}

void Map::MapMarkerModelHelper::slotDataChanged(const QModelIndex &, const QModelIndex &)
{
    emit( signalModelChangedDrastically() );
}

bool Map::MapMarkerModelHelper::itemCoordinates(const QModelIndex &index,
                                                KGeoMap::GeoCoordinates *const coordinates) const
{
    if (!index.data(FileNameRole).canConvert<DB::FileName>()) {
        return false;
    }

    if ( coordinates ) {
        const DB::FileName filename = index.data( FileNameRole ).value<DB::FileName>();
        *coordinates = KGeoMap::GeoCoordinates( filename.info()->coordinates().lat(),
                                                filename.info()->coordinates().lon() );
    }

    return true;
}

QAbstractItemModel *Map::MapMarkerModelHelper::model() const
{
    return m_itemModel;
}

QItemSelectionModel *Map::MapMarkerModelHelper::selectionModel() const
{
    return m_itemSelectionModel;
}

KGeoMap::ModelHelper::Flags Map::MapMarkerModelHelper::modelFlags() const
{
    return FlagVisible;
}

KGeoMap::ModelHelper::Flags Map::MapMarkerModelHelper::itemFlags( const QModelIndex& index ) const
{
    if (!index.data(FileNameRole).canConvert<DB::FileName>()) {
        return FlagNull;
    }

    return FlagVisible;
}

// FIXME: for some reason, itemIcon is never called -> no thumbnails
bool Map::MapMarkerModelHelper::itemIcon(const QModelIndex &index,
                                         QPoint *const offset,
                                         QSize *const,
                                         QPixmap *const pixmap,
                                         QUrl *const) const
{
    if (!index.data(FileNameRole).canConvert<DB::FileName>()) {
        return false;
    }

    const DB::FileName filename = index.data(FileNameRole).value<DB::FileName>();
    *pixmap = ImageManager::ThumbnailCache::instance()->lookup(filename);
    *offset = QPoint(pixmap->width() / 2, pixmap->height() / 2);
    qCDebug(MapLog) << "Map icon for " << filename.relative() << (pixmap->isNull() ? " missing." : " found.");
    return !pixmap->isNull();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

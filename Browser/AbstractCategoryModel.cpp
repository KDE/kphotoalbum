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

#include "AbstractCategoryModel.h"
#include "BrowserWidget.h"
#include <QApplication>
#include <KLocalizedString>
#include <DB/ImageDB.h>
#include <DB/MemberMap.h>
#include <QIcon>
#include "enums.h"

Browser::AbstractCategoryModel::AbstractCategoryModel( const DB::CategoryPtr& category, const DB::ImageSearchInfo& info )
    : m_category( category ), m_info( info )
{
    m_images = DB::ImageDB::instance()->classify( info, m_category->name(), DB::Image );
    m_videos = DB::ImageDB::instance()->classify( info, m_category->name(), DB::Video );
}

bool Browser::AbstractCategoryModel::hasNoneEntry() const
{
    int imageCount = m_images[DB::ImageDB::NONE()].count;
    int videoCount = m_videos[DB::ImageDB::NONE()].count;
    return (imageCount + videoCount != 0);
}

QString Browser::AbstractCategoryModel::text( const QString& name ) const
{
    if ( name == DB::ImageDB::NONE() ) {
        if ( m_info.categoryMatchText(m_category->name()).length() == 0 )
            return i18nc("As in No persons, no locations etc.", "None" );
        else
            return i18nc("As in no other persons, or no other locations. ", "No other" );
    }

    else {
        if (m_category->type() == DB::Category::FolderCategory) {
            QRegExp rx( QString::fromLatin1( "(.*/)(.*)$") );
            QString value = name;
            value.replace( rx, QString::fromLatin1("\\2") );
            return value;
        } else {
            return name;
        }
    }
}

QPixmap Browser::AbstractCategoryModel::icon( const QString& name ) const
{
    const int size = m_category->thumbnailSize();
    if ( BrowserWidget::isResizing() ) {
        QPixmap res( size, size * Settings::SettingsData::instance()->getThumbnailAspectRatio() );
        res.fill( Qt::white );
        return res;
    }

    if ( m_category->viewType() == DB::Category::TreeView || m_category->viewType() == DB::Category::IconView ) {
        if (DB::ImageDB::instance()->memberMap().isGroup(m_category->name(), name)) {
            return QIcon::fromTheme(QString::fromUtf8("folder-image")).pixmap(22);
        } else {
            return m_category->icon();
        }
    }
    else {
        // The category images are screenshot from the size of the viewer (Which might very well be considered a bug)
        // This is the reason for asking for the thumbnail height being 3/4 of its width.
        return m_category->categoryImage( m_category->name(), name, size, size * Settings::SettingsData::instance()->getThumbnailAspectRatio() );
    }
}

QVariant Browser::AbstractCategoryModel::data( const QModelIndex & index, int role) const
{
    if ( !index.isValid() )
        return QVariant();
    const QString name = indexToName( index );
    const int column = index.column();

    if ( role == Qt::DisplayRole ) {
        switch( column ) {
        case 0: return text(name);
        case 1: return i18ncp("@item:intable number of images with a specific tag.","1 image", "%1 images", m_images[name].count);
        case 2: return i18ncp("@item:intable number of videos with a specific tag.","1 video", "%1 videos", m_videos[name].count);
        case 3: {
            DB::ImageDate range = m_images[name].range;
            range.extendTo(m_videos[name].range);
            return range.toString(false);
        }
        }
    }

    else if ( role == Qt::DecorationRole && column == 0) {
        return icon( name );
    }

    else if ( role == Qt::ToolTipRole )
        return text(name);

    else if ( role == ItemNameRole )
        return name;

    else if ( role == ValueRole ) {
        switch ( column ) {
        case 0: return name; // Notice we sort by **None** rather than None, which makes it show up at the top for less than searches.
        case 1: return m_images[name].count;
        case 2: return m_videos[name].count;
        case 3: {
            DB::ImageDate range = m_images[name].range;
            range.extendTo(m_videos[name].range);
            return range.start().toSecsSinceEpoch();
        }
        }
    }

    return QVariant();
}

Qt::ItemFlags Browser::AbstractCategoryModel::flags( const QModelIndex& index ) const
{
    return index.isValid() ? Qt::ItemIsSelectable | Qt::ItemIsEnabled : Qt::ItemFlags();
}

QVariant Browser::AbstractCategoryModel::headerData( int section, Qt::Orientation orientation, int role) const
{
    if ( orientation == Qt::Vertical || role != Qt::DisplayRole )
        return QVariant();

    switch ( section ) {
    case 0: return m_category->name();
    case 1: return i18n("Images");
    case 2: return i18n("Videos");
    case 3: return i18n("Date range");
    }

    return QVariant();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

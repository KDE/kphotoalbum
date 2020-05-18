/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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

#include "CategoryItem.h"

// Qt includes
#include <QDir>

// KDE includes

// Local includes
#include "SettingsData.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/MemberMap.h>
#include <MainWindow/DirtyIndicator.h>
#include <MainWindow/Window.h>

Settings::CategoryItem::CategoryItem(const QString &category,
                                     const QString &icon,
                                     DB::Category::ViewType type,
                                     int thumbnailSize,
                                     QListWidget *parent,
                                     bool positionable)
    : QListWidgetItem(category, parent)
    , m_categoryOrig(category)
    , m_iconOrig(icon)
    , m_positionable(positionable)
    , m_positionableOrig(positionable)
    , m_category(category)
    , m_icon(icon)
    , m_type(type)
    , m_typeOrig(type)
    , m_thumbnailSize(thumbnailSize)
    , m_thumbnailSizeOrig(thumbnailSize)
{
    setFlags(flags() | Qt::ItemIsEditable);
}

void Settings::CategoryItem::setLabel(const QString &label)
{
    setText(label);
    m_category = label;
}

void Settings::CategoryItem::submit(DB::MemberMap *memberMap)
{
    if (m_categoryOrig.isNull()) {
        // New Item
        DB::ImageDB::instance()->categoryCollection()->addCategory(m_category,
                                                                   m_icon,
                                                                   m_type,
                                                                   m_thumbnailSize,
                                                                   true);
        MainWindow::DirtyIndicator::markDirty();
    } else {
        DB::CategoryPtr category = DB::ImageDB::instance()->categoryCollection()->categoryForName(m_categoryOrig);

        if (m_category != m_categoryOrig) {
            renameCategory(memberMap);
        }

        if (m_positionable != m_positionableOrig) {
            category->setPositionable(m_positionable);
        }

        if (m_icon != m_iconOrig) {
            category->setIconName(m_icon);
        }

        if (m_type != m_typeOrig) {
            category->setViewType(m_type);
        }

        if (m_thumbnailSize != m_thumbnailSizeOrig) {
            category->setThumbnailSize(m_thumbnailSize);
        }
    }

    m_categoryOrig = m_category;
    m_iconOrig = m_icon;
    m_typeOrig = m_type;
    m_thumbnailSizeOrig = m_thumbnailSize;
    m_positionableOrig = m_positionable;
}

void Settings::CategoryItem::removeFromDatabase()
{
    if (!m_categoryOrig.isNull()) {
        // The database knows about the item.
        DB::ImageDB::instance()->categoryCollection()->removeCategory(m_categoryOrig);
    }
}

bool Settings::CategoryItem::positionable() const
{
    return m_positionable;
}

void Settings::CategoryItem::setPositionable(bool positionable)
{
    m_positionable = positionable;
}

QString Settings::CategoryItem::icon() const
{
    return m_icon;
}

int Settings::CategoryItem::thumbnailSize() const
{
    return m_thumbnailSize;
}

DB::Category::ViewType Settings::CategoryItem::viewType() const
{
    return m_type;
}

void Settings::CategoryItem::setIcon(const QString &icon)
{
    m_icon = icon;
}

void Settings::CategoryItem::setThumbnailSize(int size)
{
    m_thumbnailSize = size;
}

void Settings::CategoryItem::setViewType(DB::Category::ViewType type)
{
    m_type = type;
}

void Settings::CategoryItem::renameCategory(DB::MemberMap *memberMap)
{
    QDir dir(QString::fromLatin1("%1/CategoryImages").arg(Settings::SettingsData::instance()->imageDirectory()));

    const QStringList files = dir.entryList(QStringList()
                                            << QString::fromLatin1("%1*").arg(m_categoryOrig));

    for (QStringList::ConstIterator fileNameIt = files.begin();
         fileNameIt != files.end();
         ++fileNameIt) {

        QString newName = m_category + (*fileNameIt).mid(m_categoryOrig.length());
        dir.rename(*fileNameIt, newName);
    }

    // update category names for privacy-lock settings:
    Settings::SettingsData *settings = Settings::SettingsData::instance();
    DB::ImageSearchInfo info = DB::ImageSearchInfo::loadLock(settings->currentLock());
    const bool exclude = settings->lockExcludes();

    info.renameCategory(m_categoryOrig, m_category);
    settings->setCurrentLock(info.getLockData(), exclude);

    DB::ImageDB::instance()->categoryCollection()->rename(m_categoryOrig, m_category);
    memberMap->renameCategory(m_categoryOrig, m_category);

    m_categoryOrig = m_category;
}

QString Settings::CategoryItem::originalName() const
{
    return m_categoryOrig;
}

void Settings::CategoryItem::markAsNewCategory()
{
    m_categoryOrig = QString();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

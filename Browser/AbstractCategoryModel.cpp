// SPDX-FileCopyrightText: 2009 Yuri Chornoivan <yurchor@ukr.net>
// SPDX-FileCopyrightText: 2009-2010 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2009-2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2011 Andreas Neustifter <andreas.neustifter@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2015-2016 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "AbstractCategoryModel.h"

#include "enums.h"

#include <DB/ImageDB.h>
#include <DB/MemberMap.h>
#include <kpabase/SettingsData.h>

#include <KLocalizedString>
#include <QApplication>
#include <QIcon>

Browser::AbstractCategoryModel::AbstractCategoryModel(const DB::CategoryPtr &category, const DB::ImageSearchInfo &info)
    : m_category(category)
    , m_info(info)
{
    m_images = DB::ImageDB::instance()->classify(info, m_category->name(), DB::Image);
    m_videos = DB::ImageDB::instance()->classify(info, m_category->name(), DB::Video);
}

bool Browser::AbstractCategoryModel::hasNoneEntry() const
{
    int imageCount = m_images[DB::ImageDB::NONE()].count;
    int videoCount = m_videos[DB::ImageDB::NONE()].count;
    return (imageCount + videoCount != 0);
}

QString Browser::AbstractCategoryModel::text(const QString &name) const
{
    if (name == DB::ImageDB::NONE()) {
        if (m_info.categoryMatchText(m_category->name()).length() == 0)
            return i18nc("As in No persons, no locations etc.", "None");
        else
            return i18nc("As in no other persons, or no other locations. ", "No other");
    }

    else {
        if (m_category->type() == DB::Category::FolderCategory) {
            QRegExp rx(QString::fromLatin1("(.*/)(.*)$"));
            QString value = name;
            value.replace(rx, QString::fromLatin1("\\2"));
            return value;
        } else {
            return name;
        }
    }
}

QPixmap Browser::AbstractCategoryModel::icon(const QString &name) const
{
    int size = m_category->thumbnailSize();
    if (m_category->viewType() == DB::Category::TreeView) {
        // for generic tree view, icons are less important and carry few information
        // Maybe we should query the system for some sensible value here somehow, but I didn't find
        // anything reasonable during a cursory search and 22px has been hardcoded in some parts of kphotoalbum
        // for this kind of items without complaints so far...
        size = 22;
    }

    if (m_category->viewType() == DB::Category::TreeView || m_category->viewType() == DB::Category::IconView) {
        if (DB::ImageDB::instance()->memberMap().isGroup(m_category->name(), name)) {
            return QIcon::fromTheme(QString::fromUtf8("folder-image")).pixmap(size);
        } else {
            return m_category->icon(size);
        }
    } else {
        // The category images are screenshot from the size of the viewer (Which might very well be considered a bug)
        return m_category->categoryImage(m_category->name(), name, size, size * Settings::SettingsData::instance()->getThumbnailAspectRatio());
    }
}

QVariant Browser::AbstractCategoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    const QString name = indexToName(index);
    const int column = index.column();

    if (role == Qt::DisplayRole) {
        switch (column) {
        case 0:
            return text(name);
        case 1:
            return i18ncp("@item:intable number of images with a specific tag.", "1 image", "%1 images", m_images[name].count);
        case 2:
            return i18ncp("@item:intable number of videos with a specific tag.", "1 video", "%1 videos", m_videos[name].count);
        case 3: {
            DB::ImageDate range = m_images[name].range;
            range.extendTo(m_videos[name].range);
            return DB::ImageDate(range.start()).toString(false);
        }
        case 4: {
            DB::ImageDate range = m_images[name].range;
            range.extendTo(m_videos[name].range);
            return DB::ImageDate(range.end()).toString(false);
        }
        }
    }

    else if (role == Qt::DecorationRole && column == 0) {
        return icon(name);
    }

    else if (role == Qt::ToolTipRole)
        return text(name);

    else if (role == ItemNameRole)
        return name;

    else if (role == ValueRole) {
        switch (column) {
        case 0:
            return name;
        case 1:
            return m_images[name].count;
        case 2:
            return m_videos[name].count;
        case 3: {
            DB::ImageDate range = m_images[name].range;
            range.extendTo(m_videos[name].range);
            return range.start().toSecsSinceEpoch();
        }
        case 4: {
            DB::ImageDate range = m_images[name].range;
            range.extendTo(m_videos[name].range);
            return range.end().toSecsSinceEpoch();
        }
        }
    } else if (role == SortPriorityRole) {
        switch (column) {
        case 0:
            // none is to be sorted first
            return (name == DB::ImageDB::NONE()) ? -1 : 0;
        default:
            return 0;
        }
    }

    return QVariant();
}

Qt::ItemFlags Browser::AbstractCategoryModel::flags(const QModelIndex &index) const
{
    return index.isValid() ? Qt::ItemIsSelectable | Qt::ItemIsEnabled : Qt::ItemFlags();
}

QVariant Browser::AbstractCategoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical || role != Qt::DisplayRole)
        return QVariant();

    switch (section) {
    case 0:
        return m_category->name();
    case 1:
        return i18n("Images");
    case 2:
        return i18n("Videos");
    case 3:
        return i18n("Start Date");
    case 4:
        return i18n("End Date");
    }

    return QVariant();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

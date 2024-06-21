// SPDX-FileCopyrightText: 2004-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2007 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2013-2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2014-2014 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Category.h"

#include "CategoryItem.h"
#include "ImageDB.h"
#include "MemberMap.h"
#include "TagInfo.h"
#include "Utilities/List.h"

#include <Utilities/ImageUtil.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpabase/UIDelegate.h>

#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QPixmap>
#include <QPixmapCache>
#include <kiconloader.h>

#include <utility>

using Utilities::StringSet;

DB::Category::Category(const QString &name, const QString &icon, ViewType type, int thumbnailSize, bool show, bool positionable)
    : m_name(name)
    , m_icon(icon)
    , m_show(show)
    , m_type(type)
    , m_thumbnailSize(thumbnailSize)
    , m_positionable(positionable)
    , m_categoryType(DB::Category::PlainCategory)
    , m_shouldSave(true)
{
}

QString DB::Category::name() const
{
    return m_name;
}

void DB::Category::setName(const QString &name)
{
    m_name = name;
}

bool DB::Category::positionable() const
{
    return m_positionable;
}

void DB::Category::setPositionable(bool positionable)
{
    if (positionable == m_positionable)
        return;

    m_positionable = positionable;
    Q_EMIT changed();
}

QString DB::Category::iconName() const
{
    return m_icon;
}

void DB::Category::setIconName(const QString &name)
{
    if (m_icon == name)
        return;

    m_icon = name;
    Q_EMIT changed();
}

QPixmap DB::Category::icon(int size, KIconLoader::States state) const
{
    QPixmap pixmap = KIconLoader::global()->loadIcon(iconName(), KIconLoader::Desktop, size, state, QStringList(), nullptr, true);
    return pixmap;
}

DB::Category::ViewType DB::Category::viewType() const
{
    return m_type;
}

void DB::Category::setViewType(ViewType type)
{
    if (m_type == type)
        return;

    m_type = type;
    Q_EMIT changed();
}

int DB::Category::thumbnailSize() const
{
    return m_thumbnailSize;
}

void DB::Category::setThumbnailSize(int size)
{
    if (m_thumbnailSize == size)
        return;

    m_thumbnailSize = size;
    Q_EMIT changed();
}

bool DB::Category::doShow() const
{
    return m_show;
}

void DB::Category::setDoShow(bool b)
{
    if (b == m_show)
        return;

    m_show = b;
    Q_EMIT changed();
}

DB::Category::CategoryType DB::Category::type() const
{
    return m_categoryType;
}

void DB::Category::setType(CategoryType t)
{
    m_categoryType = t;
}

bool DB::Category::isSpecialCategory() const
{
    return m_categoryType != DB::Category::PlainCategory;
}

QStringList DB::Category::items() const
{
    return m_items;
}

QStringList DB::Category::itemsInclCategories() const
{
    // values including member groups

    QStringList items = this->items();

    // add the groups to the list too, but only if the group is not there already, which will be the case
    // if it has ever been selected once.
    QStringList groups = DB::ImageDB::instance()->memberMap().groups(name());
    for (QStringList::ConstIterator it = groups.constBegin(); it != groups.constEnd(); ++it) {
        if (!items.contains(*it))
            items << *it;
    };

    return items;
}

DB::CategoryItem *createItem(const QString &categoryName, const QString &itemName, StringSet handledItems,
                             QMap<QString, DB::CategoryItem *> &categoryItems,
                             QMap<QString, DB::CategoryItem *> &potentialToplevelItems)
{
    handledItems.insert(itemName);
    DB::CategoryItem *result = new DB::CategoryItem(itemName);

    const QStringList members = DB::ImageDB::instance()->memberMap().members(categoryName, itemName, false);
    for (QStringList::ConstIterator memberIt = members.constBegin(); memberIt != members.constEnd(); ++memberIt) {
        if (!handledItems.contains(*memberIt)) {
            DB::CategoryItem *child;
            if (categoryItems.contains(*memberIt))
                child = categoryItems[*memberIt]->clone();
            else
                child = createItem(categoryName, *memberIt, handledItems, categoryItems, potentialToplevelItems);

            potentialToplevelItems.remove(*memberIt);
            result->mp_subcategories.append(child);
        }
    }

    categoryItems.insert(itemName, result);
    return result;
}

DB::CategoryItemPtr DB::Category::itemsCategories() const
{
    const MemberMap &map = ImageDB::instance()->memberMap();
    const QStringList groups = map.groups(name());

    QMap<QString, DB::CategoryItem *> categoryItems;
    QMap<QString, DB::CategoryItem *> potentialToplevelItems;

    for (QStringList::ConstIterator groupIt = groups.constBegin(); groupIt != groups.constEnd(); ++groupIt) {
        if (!categoryItems.contains(*groupIt)) {
            StringSet handledItems;
            DB::CategoryItem *child = createItem(name(), *groupIt, handledItems, categoryItems, potentialToplevelItems);
            potentialToplevelItems.insert(*groupIt, child);
        }
    }

    CategoryItem *result = new CategoryItem(QString::fromLatin1("top"), true);
    for (QMap<QString, DB::CategoryItem *>::ConstIterator toplevelIt = potentialToplevelItems.constBegin(); toplevelIt != potentialToplevelItems.constEnd(); ++toplevelIt) {
        result->mp_subcategories.append(*toplevelIt);
    }

    // Add items not found yet.
    QStringList elms = items();
    for (QStringList::ConstIterator elmIt = elms.constBegin(); elmIt != elms.constEnd(); ++elmIt) {
        if (!categoryItems.contains(*elmIt))
            result->mp_subcategories.append(new DB::CategoryItem(*elmIt));
    }

    return CategoryItemPtr(result);
}

void DB::Category::addOrReorderItems(const QStringList &items)
{
    m_items = Utilities::mergeListsUniqly(items, m_items);
}

void DB::Category::setItems(const QStringList &items)
{
    m_items = items;
}

void DB::Category::removeItem(const QString &item)
{
    m_items.removeAll(item);
    m_nameMap.remove(idForName(item));
    m_idMap.remove(item);
    Q_EMIT itemRemoved(item);
}

void DB::Category::renameItem(const QString &oldValue, const QString &newValue)
{
    const auto sanitizedNewValue = newValue.trimmed();
    int id = idForName(oldValue);
    m_items.removeAll(oldValue);
    m_nameMap.remove(id);
    m_idMap.remove(oldValue);

    addItem(sanitizedNewValue);
    if (id > 0)
        setIdMapping(sanitizedNewValue, id);
    Q_EMIT itemRenamed(oldValue, sanitizedNewValue);
}

void DB::Category::addItem(const QString &item)
{
    const auto sanitizedItem = item.trimmed();
    // for the "SortLastUsed" functionality in ListSelect we remove the item and insert it again:
    if (m_items.contains(sanitizedItem))
        m_items.removeAll(sanitizedItem);
    m_items.prepend(sanitizedItem);
}

DB::TagInfo *DB::Category::itemForName(const QString &tag)
{
    if (m_items.contains(tag)) {
        return new DB::TagInfo(this, tag);
    } else {
        return nullptr;
    }
}

QPixmap DB::Category::categoryImage(const QString &category, const QString &member, int width, int height) const
{
    QString fileName = fileForCategoryImage(category, member);
    QString key = QString::fromLatin1("%1-%2").arg(width).arg(fileName);
    QPixmap res;
    if (QPixmapCache::find(key, &res))
        return res;

    QImage img;
    bool ok = img.load(fileName, "JPEG");
    if (!ok) {
        if (DB::ImageDB::instance()->memberMap().isGroup(category, member))
            img = KIconLoader::global()->loadIcon(QString::fromLatin1("kuser"), KIconLoader::Desktop, qMax(width, height)).toImage();
        else
            img = icon(qMax(width, height)).toImage();
    }
    res = QPixmap::fromImage(Utilities::scaleImage(img, QSize(width, height), Qt::KeepAspectRatio));

    QPixmapCache::insert(key, res);
    return res;
}

void DB::Category::setCategoryImage(const QString &category, const QString &member, const QImage &image)
{
    QString dir = Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("CategoryImages");
    QFileInfo fi(dir);
    bool ok;
    if (!fi.exists()) {
        bool ok = QDir().mkdir(dir);
        if (!ok) {
            DB::ImageDB::instance()->uiDelegate().error(
                DB::LogMessage { DBLog(), QString::fromLatin1("Unable to create CategoryImages folder!") },
                i18n("Unable to create folder '%1'.", dir), i18n("Unable to Create Folder"));
            return;
        }
    }
    QString fileName = fileForCategoryImage(category, member);
    ok = image.save(fileName, "JPEG");
    if (!ok) {
        DB::ImageDB::instance()->uiDelegate().error(
            DB::LogMessage { DBLog(), QString::fromLatin1("Unable to save category image '%1'!").arg(fileName) },
            i18n("Error when saving image '%1'.", fileName), i18n("Error Saving Image"));
        return;
    }

    QPixmapCache::clear();
}

QString DB::Category::fileForCategoryImage(const QString &category, QString member) const
{
    QString dir = Settings::SettingsData::instance()->imageDirectory() + QString::fromLatin1("CategoryImages");
    member.replace(QChar::fromLatin1(' '), QChar::fromLatin1('_'));
    member.replace(QChar::fromLatin1('/'), QChar::fromLatin1('_'));
    QString fileName = dir + QString::fromLatin1("/%1-%2.jpg").arg(category, member);
    return fileName;
}

QDate DB::Category::birthDate(const QString &item) const
{
    return m_birthDates[item];
}

void DB::Category::setBirthDate(const QString &item, const QDate &birthDate)
{
    m_birthDates.insert(item, birthDate);
}

int DB::Category::idForName(const QString &name) const
{
    Q_ASSERT(m_idMap.count(name) <= 1);
    return m_idMap[name];
}

void DB::Category::initIdMap()
{
    // find maximum id
    // obviously, this will leave gaps in numbering when tags are deleted
    // assuming that tags are seldomly removed this should not be a problem
    int i = 1;
    if (!m_nameMap.empty()) {
        i = m_nameMap.lastKey();
    }

    for (const QString &tag : std::as_const(m_items)) {
        if (!m_idMap.contains(tag))
            setIdMapping(tag, ++i);
    }

    const QStringList groups = DB::ImageDB::instance()->memberMap().groups(m_name);
    for (const QString &group : groups) {
        if (!m_idMap.contains(group))
            setIdMapping(group, ++i);
    }
}

void DB::Category::setIdMapping(const QString &name, int id)
{
    Q_ASSERT(id > 0);
    m_nameMap.insert(id, name);
    m_idMap.insert(name, id);
}

void DB::Category::addZeroMapping(const QString &name)
{
    m_namesWithIdZero += name;
    m_idMap.insert(name, 0);
}

QString DB::Category::nameForId(int id) const
{
    Q_ASSERT(m_nameMap.count(id) <= 1);
    return m_nameMap[id];
}

QStringList DB::Category::namesForIdZero() const
{
    return m_namesWithIdZero;
}

void DB::Category::clearNullIds()
{
    for (const auto &tag : m_namesWithIdZero) {
        m_idMap.remove(tag);
    }
    m_namesWithIdZero.clear();
}

bool DB::Category::shouldSave()
{
    return m_shouldSave;
}

void DB::Category::setShouldSave(bool b)
{
    m_shouldSave = b;
}

#include "moc_Category.cpp"
// vi:expandtab:tabstop=4 shiftwidth=4:

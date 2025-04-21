// SPDX-FileCopyrightText: 2004-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2006 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2008 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2015-2025 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "CategoryCollection.h"
#include "kpabase/Logging.h"

#include <DB/ImageDB.h>

#include <utility>

DB::CategoryCollection::~CategoryCollection() = default;

DB::CategoryPtr DB::CategoryCollection::categoryForName(const QString &name) const
{
    for (QList<DB::CategoryPtr>::ConstIterator it = m_categories.begin(); it != m_categories.end(); ++it) {
        if ((*it)->name() == name)
            return *it;
    }
    return DB::CategoryPtr();
}

QStringList DB::CategoryCollection::categoryNames(IncludeSpecialCategories include) const
{
    QStringList res;
    for (const auto &category : m_categories) {
        if (include == IncludeSpecialCategories::Yes || !category->isSpecialCategory())
            res.append(category->name());
    }
    return res;
}

QList<DB::CategoryPtr> DB::CategoryCollection::categories() const
{
    return m_categories;
}

DB::CategoryPtr DB::CategoryCollection::categoryForSpecial(const Category::CategoryType type) const
{
    return m_specialCategories[type];
}

void DB::CategoryCollection::addCategory(CategoryPtr category)
{
    m_categories.append(category);
    if (category->isSpecialCategory()) {
        m_specialCategories[category->type()] = category;
    }

    connect(category.data(), &DB::Category::changed, this, &DB::CategoryCollection::categoryCollectionChanged);
    connect(category.data(), &DB::Category::itemRemoved, this, &DB::CategoryCollection::slotItemRemoved);
    connect(category.data(), &DB::Category::itemRenamed, this, &DB::CategoryCollection::slotItemRenamed);
    Q_EMIT categoryCollectionChanged();
}

void DB::CategoryCollection::addCategory(const QString &text, const QString &icon, Category::ViewType type, int thumbnailSize, bool show, bool positionable)
{
    addCategory(DB::CategoryPtr(new DB::Category(text, -1, icon, type, thumbnailSize, show, positionable)));
}

void DB::CategoryCollection::removeCategory(const QString &name)
{
    for (QList<DB::CategoryPtr>::iterator it = m_categories.begin(); it != m_categories.end(); ++it) {
        if ((*it)->name() == name) {
            m_categories.erase(it);
            qCDebug(DBLog) << "CategoryCollection::removeCategory: category" << name << "removed.";
            Q_EMIT categoryRemoved(name);
            Q_EMIT categoryCollectionChanged();
            return;
        }
    }
    qCWarning(DBLog) << "CategoryCollection::removeCategory: category" << name << "does not exist!";
    Q_ASSERT_X(false, "removeCategory", "trying to remove non-existing category");
}

void DB::CategoryCollection::rename(const QString &oldName, const QString &newName)
{
    categoryForName(oldName)->setName(newName);
    DB::ImageDB::instance()->renameCategory(oldName, newName);
    Q_EMIT categoryCollectionChanged();
}

DB::GlobalCategorySortOrder *DB::CategoryCollection::globalSortOrder()
{
    return m_globalSortOrder.get();
}

void DB::CategoryCollection::initIdMap()
{
    // Generate a sorted list of all existing IDs and the categories having them
    QMap<int, QString> ids;
    for (const auto &category : std::as_const(m_categories)) {
        const auto id = category->id();
        if (id > 0 && !ids.contains(id)) {
            ids.insert(id, category->name());
        }
    }

    // Make sure lastKey() will work and we start counting at 1
    ids.insert(0, QString());

    QList<DB::CategoryPtr> newIdNeeded;

    for (auto category : std::as_const(m_categories)) {
        // Initialize the tag IDs
        category->initIdMap();

        // Check the category ID
        if (category->needsId()) {
            if (category->id() <= 0) {
                newIdNeeded.append(category);
                qCDebug(DBLog) << "Assigning a new ID for category" << category->name();
            } else if (ids.contains(category->id()) && ids.value(category->id()) != category->name()) {
                newIdNeeded.append(category);
                qCWarning(DBLog) << "Duplicate ID" << category->id() << "used for category"
                                 << category->name() << "- assigning a new ID!";
            }
        }
    }

    for (auto category : std::as_const(newIdNeeded)) {
        const auto id = ids.lastKey() + 1;
        category->setId(id);
        qCDebug(DBLog) << "Category" << category->name() << "now has ID" << id;
        ids.insert(id, category->name());
    }
}

void DB::CategoryCollection::slotItemRenamed(const QString &oldName, const QString &newName)
{
    Q_EMIT itemRenamed(static_cast<Category *>(const_cast<QObject *>(sender())), oldName, newName);
}

void DB::CategoryCollection::slotItemRemoved(const QString &item)
{
    Q_EMIT itemRemoved(static_cast<Category *>(const_cast<QObject *>(sender())), item);
}

DB::CategoryCollection::CategoryCollection()
    : m_globalSortOrder(new GlobalCategorySortOrder())
{
}

#include "moc_CategoryCollection.cpp"
// vi:expandtab:tabstop=4 shiftwidth=4:

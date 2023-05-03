// SPDX-FileCopyrightText: 2005-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2006 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2015-2022 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "XMLCategoryCollection.h"

#include <DB/Category.h>
#include <DB/ImageDB.h>

#include <QList>

DB::CategoryPtr XMLDB::XMLCategoryCollection::categoryForName(const QString &name) const
{
    for (QList<DB::CategoryPtr>::ConstIterator it = m_categories.begin(); it != m_categories.end(); ++it) {
        if ((*it)->name() == name)
            return *it;
    }
    return DB::CategoryPtr();
}

void XMLDB::XMLCategoryCollection::addCategory(DB::CategoryPtr category)
{
    m_categories.append(category);
    if (category->isSpecialCategory()) {
        m_specialCategories[category->type()] = category;
    }
    connect(category.data(), &DB::Category::changed, this, &XMLCategoryCollection::categoryCollectionChanged);
    connect(category.data(), &DB::Category::itemRemoved, this, &XMLCategoryCollection::slotItemRemoved);
    connect(category.data(), &DB::Category::itemRenamed, this, &XMLCategoryCollection::slotItemRenamed);
    Q_EMIT categoryCollectionChanged();
}

QStringList XMLDB::XMLCategoryCollection::categoryNames(IncludeSpecialCategories include) const
{
    QStringList res;
    for (const auto &category : m_categories) {
        if (include == IncludeSpecialCategories::Yes || !category->isSpecialCategory())
            res.append(category->name());
    }
    return res;
}

void XMLDB::XMLCategoryCollection::removeCategory(const QString &name)
{
    for (QList<DB::CategoryPtr>::iterator it = m_categories.begin(); it != m_categories.end(); ++it) {
        if ((*it)->name() == name) {
            m_categories.erase(it);
            Q_EMIT categoryRemoved(name);
            Q_EMIT categoryCollectionChanged();
            return;
        }
    }
    Q_ASSERT_X(false, "removeCategory", "trying to remove non-existing category");
}

void XMLDB::XMLCategoryCollection::rename(const QString &oldName, const QString &newName)
{
    categoryForName(oldName)->setName(newName);
    DB::ImageDB::instance()->renameCategory(oldName, newName);
    Q_EMIT categoryCollectionChanged();
}

QList<DB::CategoryPtr> XMLDB::XMLCategoryCollection::categories() const
{
    return m_categories;
}

void XMLDB::XMLCategoryCollection::addCategory(const QString &text, const QString &icon,
                                               DB::Category::ViewType type, int thumbnailSize, bool show, bool positionable)
{
    addCategory(DB::CategoryPtr(new DB::Category(text, icon, type, thumbnailSize, show, positionable)));
}

DB::CategoryPtr XMLDB::XMLCategoryCollection::categoryForSpecial(const DB::Category::CategoryType type) const
{
    return m_specialCategories[type];
}

void XMLDB::XMLCategoryCollection::initIdMap()
{
    for (DB::CategoryPtr categoryPtr : qAsConst(m_categories)) {
        categoryPtr->initIdMap();
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_XMLCategoryCollection.cpp"

// SPDX-FileCopyrightText: 2003 - 2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>
// SPDX-FileCopyrightText: 2005 - 2007 Dirk Mueller <mueller@kde.org>
// SPDX-FileCopyrightText: 2006 - 2007 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2007 - 2008 Laurent Montel <montel@kde.org>
// SPDX-FileCopyrightText: 2007 - 2010 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2008 - 2009 Henner Zeller <h.zeller@acm.org>
// SPDX-FileCopyrightText: 2013 - 2024 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2018 - 2022 Tobias Leupold <tl@stonemx.de>
// SPDX-FileCopyrightText: 2018 Robert Krawitz <rlk@alum.mit.edu>
// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "MemberMap.h"

#include "Category.h"

#include <kpabase/Logging.h>

using namespace DB;

MemberMap::MemberMap()
    : QObject(nullptr)
    , m_dirty(true)
    , m_loading(false)
{
}

MemberMap::MemberMap(const MemberMap &other)
    : QObject(nullptr)
    , m_members(other.memberMap())
    , m_dirty(true)
    , m_loading(false)
{
}

MemberMap &MemberMap::operator=(const MemberMap &other)
{
    if (this != &other) {
        m_members = other.memberMap();
        m_dirty = true;
    }
    return *this;
}

QStringList MemberMap::groups(const QString &category) const
{
    return QStringList(m_members[category].keys());
}

bool MemberMap::contains(const QString &category, const QString &item) const
{
    return m_flatMembers.contains(category) && m_flatMembers[category].contains(item);
}

void MemberMap::markDirty(const QString &category)
{
    if (m_loading)
        regenerateFlatList(category);
    else
        Q_EMIT dirty();
}

void MemberMap::deleteGroup(const QString &category, const QString &groupName)
{
    if (!m_members.contains(category))
        return;

    if (m_members[category].remove(groupName) > 0) {
        m_dirty = true;
        markDirty(category);
    }
}

QStringList MemberMap::members(const QString &category, const QString &memberGroup, bool closure) const
{
    if (!m_members.contains(category)) {
        return {};
    }
    if (closure) {
        if (m_dirty) {
            calculate();
        }
        const auto &members = m_closureMembers[category][memberGroup];
        return QStringList(members.begin(), members.end());
    } else {
        const auto &members = m_members[category][memberGroup];
        return QStringList(members.begin(), members.end());
    }
}

void MemberMap::setMembers(const QString &category, const QString &memberGroup, const QStringList &members)
{
    Q_ASSERT(!category.isEmpty());
    Q_ASSERT(!memberGroup.isEmpty());
    StringSet allowedMembers(members.begin(), members.end());

    for (QStringList::const_iterator i = members.begin(); i != members.end(); ++i)
        if (!canAddMemberToGroup(category, memberGroup, *i))
            allowedMembers.remove(*i);

    m_members[category][memberGroup] = allowedMembers;
    m_dirty = true;
    markDirty(category);
}

bool MemberMap::isEmpty() const
{
    return m_members.empty();
}

bool MemberMap::isGroup(const QString &category, const QString &item) const
{
    return m_members.contains(category) && m_members[category].contains(item);
}

QMap<QString, StringSet> MemberMap::groupMap(const QString &category) const
{
    if (!m_members.contains(category))
        return {};

    if (m_dirty)
        calculate();

    return m_closureMembers[category];
}

QStringList MemberMap::calculateClosure(QMap<QString, StringSet> &resultSoFar, const QString &category, const QString &group) const
{
    resultSoFar[group] = StringSet(); // Prevent against cycles.
    const StringSet members = m_members[category][group];
    StringSet result = members;
    for (const auto &member : members) {
        if (resultSoFar.contains(member)) {
            result += resultSoFar[member];
        } else if (isGroup(category, member)) {
            const auto closure = calculateClosure(resultSoFar, category, member);
            const StringSet closureSet(closure.begin(), closure.end());
            result += closureSet;
        }
    }

    resultSoFar[group] = result;
    return QStringList(result.begin(), result.end());
}

void MemberMap::calculate() const
{
    m_closureMembers.clear();
    // run through all categories
    for (QMap<QString, QMap<QString, StringSet>>::ConstIterator categoryIt = m_members.begin();
         categoryIt != m_members.end(); ++categoryIt) {

        QString category = categoryIt.key();
        QMap<QString, StringSet> groupMap = categoryIt.value();

        // Run through each of the groups for the given categories
        for (QMap<QString, StringSet>::const_iterator groupIt = groupMap.constBegin(); groupIt != groupMap.constEnd(); ++groupIt) {
            QString group = groupIt.key();
            if (m_closureMembers[category].find(group) == m_closureMembers[category].end()) {
                (void)calculateClosure(m_closureMembers[category], category, group);
            }
        }
    }
    m_dirty = false;
}

void MemberMap::renameGroup(const QString &category, const QString &oldName, const QString &newName)
{
    const auto sanitizedNewName = newName.trimmed();
    if (!m_members.contains(category))
        return;
    if (!m_members[category].contains(oldName))
        return;
    // Don't allow overwriting to avoid creating cycles
    if (m_members[category].contains(sanitizedNewName))
        return;

    m_dirty = true;
    markDirty(category);
    QMap<QString, StringSet> &groupMap = m_members[category];
    groupMap.insert(sanitizedNewName, m_members[category][oldName]);
    groupMap.remove(oldName);
    for (StringSet &set : groupMap) {
        if (set.contains(oldName)) {
            set.remove(oldName);
            set.insert(sanitizedNewName);
        }
    }
}

void MemberMap::deleteItem(DB::Category *category, const QString &name)
{
    Q_ASSERT(category != nullptr);
    const auto categoryName = category->name();
    if (!m_members.contains(categoryName))
        return;

    int removed = 0;
    QMap<QString, StringSet> &groupMap = m_members[categoryName];
    for (StringSet &items : groupMap) {
        removed += items.remove(name);
    }
    removed += m_members[categoryName].remove(name);

    if (removed > 0) {
        m_dirty = true;
        markDirty(categoryName);
    }
}

void MemberMap::renameItem(DB::Category *category, const QString &oldName, const QString &newName)
{
    Q_ASSERT(category != nullptr);
    const auto categoryName = category->name();
    const auto sanitizedNewName = newName.trimmed();
    if (!m_members.contains(categoryName))
        return;
    if (oldName == sanitizedNewName)
        return;

    bool changed = false;
    QMap<QString, StringSet> &groupMap = m_members[categoryName];
    for (StringSet &items : groupMap) {
        if (items.contains(oldName)) {
            changed = true;
            items.remove(oldName);
            items.insert(sanitizedNewName);
        }
    }
    if (groupMap.contains(oldName)) {
        changed = true;
        groupMap[sanitizedNewName] = groupMap[oldName];
        groupMap.remove(oldName);
    }

    if (changed) {
        m_dirty = true;
        markDirty(categoryName);
    }
}

void MemberMap::regenerateFlatList(const QString &category)
{
    if (!m_members.contains(category))
        return;

    m_flatMembers[category].clear();
    for (const auto &group : qAsConst(m_members[category])) {
        for (const auto &tag : group) {
            m_flatMembers[category].insert(tag);
        }
    }
}

void MemberMap::addMemberToGroup(const QString &category, const QString &group, const QString &item)
{
    // Only test for cycles after database is already loaded
    if (!m_loading && !canAddMemberToGroup(category, group, item)) {
        qCWarning(DBLog, "Inserting item %s into group %s/%s would create a cycle. Ignoring...", qPrintable(item), qPrintable(category), qPrintable(group));
        return;
    }

    if (item.isEmpty()) {
        qCWarning(DBLog, "Tried to insert null item into group %s/%s. Ignoring...", qPrintable(category), qPrintable(group));
        return;
    }

    m_members[category][group].insert(item);
    m_flatMembers[category].insert(item);

    if (m_loading) {
        m_dirty = true;
    } else if (!m_dirty) {
        // Update _closureMembers to avoid marking it dirty

        QMap<QString, StringSet> &categoryClosure = m_closureMembers[category];

        categoryClosure[group].insert(item);

        QMap<QString, StringSet>::const_iterator
            closureOfItem
            = categoryClosure.constFind(item);
        const StringSet *closureOfItemPtr(nullptr);
        if (closureOfItem != categoryClosure.constEnd()) {
            closureOfItemPtr = &(*closureOfItem);
            categoryClosure[group] += *closureOfItem;
        }

        for (QMap<QString, StringSet>::iterator i = categoryClosure.begin();
             i != categoryClosure.end(); ++i)
            if ((*i).contains(group)) {
                (*i).insert(item);
                if (closureOfItemPtr)
                    (*i) += *closureOfItemPtr;
            }
    }

    // If we are loading, we do *not* want to regenerate the list!
    if (!m_loading)
        Q_EMIT dirty();
}

void MemberMap::removeMemberFromGroup(const QString &category, const QString &group, const QString &item)
{
    if (!m_members.contains(category))
        return;
    if (!m_members[category].contains(group))
        return;

    if (m_members[category][group].remove(item) > 0) {
        // We shouldn't be doing this very often, so just regenerate
        // the flat list
        regenerateFlatList(category);
        Q_EMIT dirty();
    }
}

void MemberMap::addGroup(const QString &category, const QString &group)
{
    const auto sanitizedGroup = group.trimmed();
    if (sanitizedGroup.isEmpty())
        return;

    if (!m_members[category].contains(sanitizedGroup)) {
        m_members[category].insert(sanitizedGroup, StringSet());
        markDirty(category);
    }
}

void MemberMap::renameCategory(const QString &oldName, const QString &newName)
{
    const auto sanitizedNewName = newName.trimmed();
    if (!m_members.contains(oldName))
        return;
    if (oldName == sanitizedNewName)
        return;
    if (m_members.contains(sanitizedNewName))
        return;

    m_members[sanitizedNewName] = m_members[oldName];
    m_members.remove(oldName);
    m_closureMembers[sanitizedNewName] = m_closureMembers[oldName];
    m_closureMembers.remove(oldName);
    if (!m_loading)
        Q_EMIT dirty();
}

void MemberMap::deleteCategory(const QString &category)
{
    if (!m_members.contains(category))
        return;

    m_members.remove(category);
    m_closureMembers.remove(category);
    markDirty(category);
}

QMap<QString, StringSet> DB::MemberMap::inverseMap(const QString &category) const
{
    QMap<QString, StringSet> res;
    const QMap<QString, StringSet> &map = m_members[category];

    for (QMap<QString, StringSet>::ConstIterator mapIt = map.begin(); mapIt != map.end(); ++mapIt) {
        QString group = mapIt.key();
        const StringSet members = mapIt.value();
        for (const auto &member : members) {
            res[member].insert(group);
        }
    }
    return res;
}

bool DB::MemberMap::hasPath(const QString &category, const QString &from, const QString &to) const
{
    if (from == to)
        return true;
    else if (!m_members[category].contains(from))
        // Try to avoid calculate(), which is quite time consuming.
        return false;
    else {
        // return members(category, from, true).contains(to);
        if (m_dirty)
            calculate();
        return m_closureMembers[category][from].contains(to);
    }
}

void DB::MemberMap::setLoading(bool isLoading)
{
    if (m_loading && !isLoading) {
        // TODO: Remove possible loaded cycles.
    }
    m_loading = isLoading;
}

#include "moc_MemberMap.cpp"

// vi:expandtab:tabstop=4 shiftwidth=4:

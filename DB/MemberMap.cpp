/* SPDX-FileCopyrightText: 2003-2020 The KPhotoAlbum Development Team

   SPDX-License-Identifier: GPL-2.0-or-later
*/

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

/**
   returns the groups directly available from category (non closure that is)
*/
QStringList MemberMap::groups(const QString &category) const
{
    return QStringList(m_members[category].keys());
}

bool MemberMap::contains(const QString &category, const QString &item) const
{
    return m_flatMembers[category].contains(item);
}

void MemberMap::markDirty(const QString &category)
{
    if (m_loading)
        regenerateFlatList(category);
    else
        emit dirty();
}

void MemberMap::deleteGroup(const QString &category, const QString &name)
{
    Q_ASSERT(m_members.contains(category));
    m_members[category].remove(name);
    m_dirty = true;
    markDirty(category);
}

/**
   return all the members of memberGroup
*/
QStringList MemberMap::members(const QString &category, const QString &memberGroup, bool closure) const
{
    if (!m_members.contains(category)) {
        return {};
    }
    if (closure) {
        if (m_dirty) {
            calculate();
        }
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto &members = m_closureMembers[category][memberGroup];
        return QStringList(members.begin(), members.end());
#else
        return m_closureMembers[category][memberGroup].toList();
#endif
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        const auto &members = m_members[category][memberGroup];
        return QStringList(members.begin(), members.end());
#else
        return m_members[category][memberGroup].toList();
#endif
    }
}

void MemberMap::setMembers(const QString &category, const QString &memberGroup, const QStringList &members)
{
    Q_ASSERT(!category.isEmpty());
    Q_ASSERT(!memberGroup.isEmpty());
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    StringSet allowedMembers(members.begin(), members.end());
#else
    StringSet allowedMembers = members.toSet();
#endif

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

/**
   returns true if item is a group for category.
*/
bool MemberMap::isGroup(const QString &category, const QString &item) const
{
    return m_members[category].find(item) != m_members[category].end();
}

/**
   return a map from groupName to list of items for category
   example: { USA |-> [Chicago, Grand Canyon, Santa Clara], Denmark |-> [Esbjerg, Odense] }
*/
QMap<QString, StringSet> MemberMap::groupMap(const QString &category) const
{
    if (!m_members.contains(category))
        return {};

    if (m_dirty)
        calculate();

    return m_closureMembers[category];
}

/**
   Calculates the closure for group, that is finds all members for group.
   Imagine there is a group called USA, and that this groups has a group inside it called Califonia,
   Califonia consists of members San Fransisco and Los Angeless.
   This function then maps USA to include Califonia, San Fransisco and Los Angeless.
*/
QStringList MemberMap::calculateClosure(QMap<QString, StringSet> &resultSoFar, const QString &category, const QString &group) const
{
    resultSoFar[group] = StringSet(); // Prevent against cycles.
    StringSet members = m_members[category][group];
    StringSet result = members;
    for (StringSet::const_iterator it = members.begin(); it != members.end(); ++it) {
        if (resultSoFar.contains(*it)) {
            result += resultSoFar[*it];
        } else if (isGroup(category, *it)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            const auto closure = calculateClosure(resultSoFar, category, *it);
            const StringSet closureSet(closure.begin(), closure.end());
#else
            const StringSet closureSet = calculateClosure(resultSoFar, category, *it).toSet();
#endif
            result += closureSet;
        }
    }

    resultSoFar[group] = result;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QStringList(result.begin(), result.end());
#else
    return result.toList();
#endif
}

/**
   This methods create the map _closureMembers from _members
   This is simply to avoid finding the closure each and every time it is needed.
*/
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
    Q_ASSERT(m_members.contains(category));
    // Don't allow overwriting to avoid creating cycles
    if (m_members[category].contains(newName))
        return;

    m_dirty = true;
    markDirty(category);
    QMap<QString, StringSet> &groupMap = m_members[category];
    groupMap.insert(newName, m_members[category][oldName]);
    groupMap.remove(oldName);
    for (StringSet &set : groupMap) {
        if (set.contains(oldName)) {
            set.remove(oldName);
            set.insert(newName);
        }
    }
}

MemberMap::MemberMap(const MemberMap &other)
    : QObject(nullptr)
    , m_members(other.memberMap())
    , m_dirty(true)
    , m_loading(false)
{
}

void MemberMap::deleteItem(DB::Category *category, const QString &name)
{
    Q_ASSERT(category != nullptr);
    QMap<QString, StringSet> &groupMap = m_members[category->name()];
    for (StringSet &items : groupMap) {
        items.remove(name);
    }
    m_members[category->name()].remove(name);
    m_dirty = true;
    markDirty(category->name());
}

void MemberMap::renameItem(DB::Category *category, const QString &oldName, const QString &newName)
{
    Q_ASSERT(category != nullptr);
    if (oldName == newName)
        return;

    QMap<QString, StringSet> &groupMap = m_members[category->name()];
    for (StringSet &items : groupMap) {
        if (items.contains(oldName)) {
            items.remove(oldName);
            items.insert(newName);
        }
    }
    if (groupMap.contains(oldName)) {
        groupMap[newName] = groupMap[oldName];
        groupMap.remove(oldName);
    }
    m_dirty = true;
    markDirty(category->name());
}

MemberMap &MemberMap::operator=(const MemberMap &other)
{
    if (this != &other) {
        m_members = other.memberMap();
        m_dirty = true;
    }
    return *this;
}

void MemberMap::regenerateFlatList(const QString &category)
{
    Q_ASSERT(m_members.contains(category));
    m_flatMembers[category].clear();
    for (QMap<QString, StringSet>::const_iterator i = m_members[category].constBegin();
         i != m_members[category].constEnd(); i++) {
        for (StringSet::const_iterator j = i.value().constBegin(); j != i.value().constEnd(); j++) {
            m_flatMembers[category].insert(*j);
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
        emit dirty();
}

void MemberMap::removeMemberFromGroup(const QString &category, const QString &group, const QString &item)
{
    Q_ASSERT(m_members.contains(category));
    if (m_members[category].contains(group)) {
        m_members[category][group].remove(item);
        // We shouldn't be doing this very often, so just regenerate
        // the flat list
        regenerateFlatList(category);
        emit dirty();
    }
}

void MemberMap::addGroup(const QString &category, const QString &group)
{
    Q_ASSERT(!group.isEmpty());
    if (!m_members[category].contains(group)) {
        m_members[category].insert(group, StringSet());
    }
    markDirty(category);
}

void MemberMap::renameCategory(const QString &oldName, const QString &newName)
{
    Q_ASSERT(m_members.contains(oldName));
    if (oldName == newName)
        return;
    m_members[newName] = m_members[oldName];
    m_members.remove(oldName);
    m_closureMembers[newName] = m_closureMembers[oldName];
    m_closureMembers.remove(oldName);
    if (!m_loading)
        emit dirty();
}

void MemberMap::deleteCategory(const QString &category)
{
    Q_ASSERT(m_members.contains(category));
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
        StringSet members = mapIt.value();
        for (StringSet::const_iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt) {
            res[*memberIt].insert(group);
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

void DB::MemberMap::setLoading(bool b)
{
    if (m_loading && !b) {
        // TODO: Remove possible loaded cycles.
    }
    m_loading = b;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

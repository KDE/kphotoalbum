// SPDX-FileCopyrightText: 2003 - 2022 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef MEMBERMAP_H
#define MEMBERMAP_H
#include <kpabase/StringSet.h>

#include <qmap.h>
#include <qobject.h>
#include <qstringlist.h>

namespace DB
{
using Utilities::StringSet;

class Category;

/**
 * @brief The MemberMap class represents tag groups for all categories.
 * The MemberMap can handle direct tag group membership, and also computes the membership closure (i.e. indirect membership).
 *
 * Tag groups in a category are a directed acyclic graph (DAG).
 * All MemberMap functions preserve the validity of the DAG (after loading the database is completed).
 */
class MemberMap : public QObject
{
    Q_OBJECT
public:
    MemberMap();
    MemberMap(const MemberMap &);
    virtual MemberMap &operator=(const MemberMap &);

    // TODO: this should return a StringSet
    /**
     * @brief groups
     * @param category the category name
     * @return the groups directly available from category (non closure that is)
     */
    virtual QStringList groups(const QString &category) const;
    /**
     * @brief deleteGroup deletes the a group from the category.
     * Does nothing if category does not have any groups or groupName is not a member of category.
     * @param category the category name
     * @param groupName
     */
    virtual void deleteGroup(const QString &category, const QString &groupName);

    // TODO: this should return a StringSet
    /**
     * @brief members
     *
     * @param category the name of the category
     * @param memberGroup the name of the tag group
     * @param closure if \c true, return indirect members as well. If \c false, only return direct members.
     *
     * @return all the members of memberGroup
     */
    virtual QStringList members(const QString &category, const QString &memberGroup, bool closure) const;
    /**
     * @brief setMembers set (direct) members for a tag group, ensuring no cycles in the group membership DAG.
     *
     * If the group already exists, its contents are replaced.
     *
     * @param category a valid category name
     * @param memberGroup a tag group name
     * @param members a list of tags
     */
    virtual void setMembers(const QString &category, const QString &memberGroup, const QStringList &members);
    /**
     * @brief isEmpty
     * @return \c true, if the MemberMap is empty, \c false otherwise.
     */
    virtual bool isEmpty() const;
    /**
     * @brief isGroup
     * @param category
     * @param memberGroup
     * @return \c true if item is a group for category, \c false otherwise.
     */
    virtual bool isGroup(const QString &category, const QString &memberGroup) const;
    /**
     * @brief groupMap returns a map of all groups in the given category.
     * Note: the returned map contains direct and indirect members for each group (i.e. all closure members).
     *
     * Example: { USA |-> [Chicago, Grand Canyon, Santa Clara], Denmark |-> [Esbjerg, Odense] }
     *
     * @param category
     * @return a map from groupName to list of items for category, or an empty map if category has no groups.
     */
    virtual QMap<QString, StringSet> groupMap(const QString &category) const;

    // TODO(jzarl): document MemberMap::inverseMap
    virtual QMap<QString, StringSet> inverseMap(const QString &category) const;
    /**
     * @brief renameGroup renames a group in category.
     * Does nothing, if category does not have any groups, if oldName is not a member of category, or if newName does already exist.
     * @param category the category name
     * @param oldName
     * @param newName
     */
    virtual void renameGroup(const QString &category, const QString &oldName, const QString &newName);
    /**
     * @brief renameCategory renames the category from oldName to newName
     * Does nothing, if the oldName does not have any groups, if oldName is the same as newName, or if newName already exists.
     * @param oldName
     * @param newName
     */
    virtual void renameCategory(const QString &oldName, const QString &newName);

    /**
     * @brief addGroup adds the tag group to category.
     * Does nothing if the group name is empty.
     * @param category the category name
     * @param group the tag group name
     */
    virtual void addGroup(const QString &category, const QString &group);
    /**
     * @brief canAddMemberToGroup checks if adding a tag to a group is possible.
     * Adding a tag to a group is not possible, if doing so would create a cycle in the group membership DAG.
     * @param category
     * @param group
     * @param item
     * @return \c true, if it is possible to add a tag to the group, or \c false otherwise.
     */
    bool canAddMemberToGroup(const QString &category, const QString &group, const QString &item) const
    {
        // If there already is a path from item to group then adding the
        // item to group would create a cycle, which we don't want.
        return !hasPath(category, item, group);
    }
    virtual void addMemberToGroup(const QString &category, const QString &group, const QString &item);
    /**
     * @brief removeMemberFromGroup removes a tag from a tag group in the given category.
     * Do nothing if the category or group is not known, or if the tag does not exist.
     * @param category
     * @param group the name of the tag group
     * @param item the name of the tag
     */
    virtual void removeMemberFromGroup(const QString &category, const QString &group, const QString &item);

    /**
     * @brief memberMap
     * @return the raw member map data
     */
    virtual const QMap<QString, QMap<QString, StringSet>> &memberMap() const { return m_members; }

    /**
     * @brief hasPath checks if the group directly or indirectly contains the tag.
     *
     * @param category a category name
     * @param from a group name
     * @param to a tag name
     * @return \c true if tag \c to can be reached from the \c from group in the group membership DAG, \c false otherwise.
     */
    virtual bool hasPath(const QString &category, const QString &from, const QString &to) const;
    /**
     * @brief contains checks if item is a member of category.
     * @param category
     * @param item
     * @return \c true, if item is contained in category.
     */
    virtual bool contains(const QString &category, const QString &item) const;

protected:
    /**
     * @brief Recalculate indirect group memberships.
     *
     * This methods create the map _closureMembers from _members
     * This is to avoid finding the closure each and every time it is needed.
     *
     * This method needs to be called whenever indirect group membership is affected by an action.
     */
    void calculate() const;
    /**
     * @brief calculateClosure calculates the closure for group, that is finds all members for group.
     *
     * Imagine there is a group called USA, and that this groups has a group inside it called Califonia,
     * Califonia consists of members San Fransisco and Los Angeless.
     * This function then maps USA to include Califonia, San Fransisco and Los Angeless.
     * @param resultSoFar
     * @param category
     * @param group
     * @return
     */
    QStringList calculateClosure(QMap<QString, StringSet> &resultSoFar, const QString &category, const QString &group) const;

public Q_SLOTS:
    /**
     * @brief deleteCategory deletes the category and its groups from the MemberMap
     * Does nothing if category does not have any groups.
     * @param category
     */
    virtual void deleteCategory(const QString &category);
    /**
     * @brief deleteItem deletes a tag from the category
     * Does nothing if the category is not known, or if the tag does not exist.
     * @param category a valid pointer to a category
     * @param name name of the tag
     */
    virtual void deleteItem(DB::Category *category, const QString &name);
    /**
     * @brief renameItem renames a tag of the given category.
     * Does nothing if the category is not known, if oldName does not exist, or if the new name is the same as the old name.
     * @param category a valid pointer to a category
     * @param oldName
     * @param newName
     */
    virtual void renameItem(DB::Category *category, const QString &oldName, const QString &newName);
    /**
     * @brief setLoading tells the MemberMap to avoid costly calculations and checks while the database is loaded from disk.
     * @param isLoading
     */
    void setLoading(bool isLoading);

Q_SIGNALS:
    void dirty();

private:
    void markDirty(const QString &category);
    void regenerateFlatList(const QString &category);
    // This is the primary data structure
    // { category |-> { group |-> [ member ] } } <- VDM syntax ;-)
    QMap<QString, QMap<QString, StringSet>> m_members;
    mutable QMap<QString, QSet<QString>> m_flatMembers;

    // These are the data structures used to develop closures, they are only
    // needed to speed up the program *SIGNIFICANTLY* ;-)
    mutable bool m_dirty;
    mutable QMap<QString, QMap<QString, StringSet>> m_closureMembers;

    bool m_loading;
};
}

#endif /* MEMBERMAP_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

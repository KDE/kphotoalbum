/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#include "SQLMemberMap.h"
#include "QueryHelper.h"
#include "QueryErrors.h"
#include "Utilities/Graph.h"

using namespace SQLDB;


SQLMemberMap::SQLMemberMap(QueryHelper& queryHelper):
    _qh(queryHelper)
{
}

DB::MemberMap& SQLMemberMap::operator=(const DB::MemberMap& other)
{
    // Works also in the _rare_ case this == &other, so don't check
    overwriteWithMemberMap(other.memberMap());
    return *this;
}

QStringList SQLMemberMap::groups(const QString& category) const
{
    return
        _qh.executeQuery("SELECT tag.name FROM tag, category "
                         "WHERE tag.categoryId=category.id AND "
                         "tag.isGroup AND category.name=%s",
                         QueryHelper::Bindings() << category).asStringList();
}

void SQLMemberMap::deleteGroup(const QString& category, const QString& name)
{
    _qh.executeStatement("DELETE FROM membergroup "
                         "WHERE groupTag=(SELECT tag.id FROM tag, category "
                         "WHERE tag.categoryId=category.id AND "
                         "category.name=%s AND tag.name=%s)",
                         QueryHelper::Bindings() << category << name);
    _qh.executeStatement("UPDATE tag, category SET tag.isGroup='0' "
                         "WHERE tag.categoryId=category.id AND "
                         "category.name=%s AND tag.name=%s",
                         QueryHelper::Bindings() << category << name);
}

QStringList SQLMemberMap::members(const QString& category,
                                  const QString& memberGroup,
                                  bool closure) const
{
    if (closure) {
        QValueList<int> idList = _qh.tagIdList(category, memberGroup);
        idList.pop_front(); // the tag itself
        return
            _qh.executeQuery("SELECT name FROM tag WHERE id IN (%s)",
                             QueryHelper::Bindings() << toVariantList(idList)
                             ).asStringList();
    }
    else
        return
            _qh.executeQuery("SELECT m.name "
                             "FROM tag m, tag g, membergroup mg, category c "
                             "WHERE m.id=mg.memberTag AND "
                             "g.id=mg.groupTag AND "
                             "g.categoryId=c.id AND c.name=%s AND g.name=%s",
                             QueryHelper::Bindings() <<
                             category << memberGroup).asStringList();
}

void SQLMemberMap::setMembers(const QString& category,
                              const QString& memberGroup,
                              const QStringList& members)
{
    if (category == "Folder")
        return;

    int groupId = _qh.tagId(category, memberGroup);
    _qh.executeStatement("UPDATE tag SET isGroup='1' WHERE id=%s",
                         QueryHelper::Bindings() << groupId);
    _qh.executeStatement("DELETE FROM membergroup WHERE groupTag=%s",
                         QueryHelper::Bindings() << groupId);
    for (QStringList::const_iterator i = members.begin();
         i != members.end(); ++i) {
        int memberId = _qh.tagId(category, *i);
        _qh.executeStatement("INSERT INTO membergroup (groupTag, memberTag) "
                             "VALUES (%s, %s)",
                             QueryHelper::Bindings() << groupId << memberId);
    }
 }

bool SQLMemberMap::isEmpty() const
{
    return _qh.executeQuery("SELECT COUNT(*) FROM tag WHERE isGroup"
                            ).firstItem().toUInt() == 0;
}

bool SQLMemberMap::isGroup(const QString& category,
                           const QString& memberGroup) const
{
    return
        _qh.executeQuery("SELECT COUNT(*) FROM tag, category "
                         "WHERE tag.categoryId=category.id AND "
                         "tag.isGroup AND "
                         "tag.name=%s AND category.name=%s",
                         QueryHelper::Bindings() << memberGroup << category
                         ).firstItem().toUInt() > 0;
}

QMap<QString, StringSet>
SQLMemberMap::groupMap(const QString& category) const
{
    return closure(pairsToMap(_qh.memberGroupConfiguration(category)));
}

QMap<QString, QStringList>
SQLMemberMap::inverseMap(const QString& category) const
{
    QMap<QString, QStringList> r;

    const QValueList<QString[2]> pairs =
        _qh.memberGroupConfiguration(category);

    for (QValueList<QString[2]>::const_iterator i = pairs.begin();
         i != pairs.end(); ++i)
        r[(*i)[1]].append((*i)[0]);

    return r;
}

void SQLMemberMap::renameGroup(const QString& category,
                               const QString& oldName, const QString& newName)
{
    // Real renaming is done elsewhere. This is just called to make
    // sure membermap is updated, so there's nothing to do.
    Q_UNUSED(category);
    Q_UNUSED(oldName);
    Q_UNUSED(newName);
}

void SQLMemberMap::renameCategory(const QString& oldName,
                                  const QString& newName)
{
    // Real renaming is done elsewhere. This is just called to make
    // sure membermap is updated, so there's nothing to do.
    Q_UNUSED(oldName);
    Q_UNUSED(newName);
}


void SQLMemberMap::addGroup(const QString& category, const QString& group)
{
    if (category == "Folder")
        return;

    if (_qh.executeQuery("SELECT COUNT(*) FROM tag, category "
                         "WHERE tag.categoryId=category.id AND "
                         "category.name=%s AND tag.name=%s",
                         QueryHelper::Bindings() << category << group
                         ).firstItem().toUInt() == 0)
        _qh.insertTagFirst(_qh.categoryId(category), group);

    _qh.executeStatement("UPDATE tag, category SET tag.isGroup='1' "
                         "WHERE tag.categoryId=category.id AND "
                         "category.name=%s AND tag.name=%s",
                         QueryHelper::Bindings() << category << group);
}

void SQLMemberMap::addMemberToGroup(const QString& category,
                                    const QString& group, const QString& item)
{
    if (category == "Folder")
        return;

    int groupId;
    int memberId;
    try {
        groupId = _qh.tagId(category, group);
    }
    catch (EntryNotFoundError&) {
        addGroup(category, group);
        groupId = _qh.tagId(category, group);
    }
    try {
        memberId = _qh.tagId(category, item);
    }
    catch (EntryNotFoundError&) {
        _qh.insertTagFirst(_qh.categoryId(category), item);
        memberId = _qh.tagId(category, item);
    }
    if (_qh.executeQuery("SELECT COUNT(*) FROM membergroup "
                         "WHERE memberTag=%s AND groupTag=%s",
                         QueryHelper::Bindings() << memberId << groupId
                         ).firstItem().toUInt() == 0)
        _qh.executeStatement("INSERT INTO membergroup (groupTag, memberTag) "
                             "VALUES (%s, %s)", QueryHelper::Bindings() <<
                             groupId << memberId);
}

void SQLMemberMap::removeMemberFromGroup(const QString& category,
                                         const QString& group,
                                         const QString& item)
{
    if (category == "Folder")
        return;

    int groupId = _qh.tagId(category, group);
    int memberId = _qh.tagId(category, item);
    _qh.executeStatement("DELETE FROM membergroup"
                         "WHERE groupTag=%s AND memberTag=%s",
                         QueryHelper::Bindings() << groupId << memberId);
}

const SQLMemberMap::MemberMapping& SQLMemberMap::memberMap() const
{
    updateMemberMapCache();
    return _memberMapCache;
}

void SQLMemberMap::updateMemberMapCache() const
{
    _memberMapCache.clear();
    QValueList<QString[3]> l = _qh.memberGroupConfiguration();
    for (QValueList<QString[3]>::const_iterator i = l.begin();
         i != l.end(); ++i)
        _memberMapCache[(*i)[0]][(*i)[1]].insert((*i)[2]);
}

void SQLMemberMap::deleteItem(DB::Category* category, const QString& name)
{
    Q_UNUSED(category);
    Q_UNUSED(name);
}

void SQLMemberMap::renameItem(DB::Category* category,
                              const QString& oldName, const QString& newName)
{
    Q_UNUSED(category);
    Q_UNUSED(oldName);
    Q_UNUSED(newName);
}

void SQLMemberMap::overwriteWithMemberMap(const MemberMapping& map)
{
    _qh.executeStatement("DELETE FROM membergroup");

    for (MemberMapping::const_iterator i = map.begin(); i != map.end(); ++i) {
        QString category = i.key();
        if (category == "Folder")
            continue;
        CategoryGroups groups = i.data();

        for (CategoryGroups::const_iterator j = groups.begin();
             j != groups.end(); ++j) {
            QString group = j.key();
            StringSet members = j.data();

            int groupId;
            try {
                groupId = _qh.tagId(category, group);
            }
            catch (EntryNotFoundError&) {
                // TODO: this
                qDebug("NYI: blaa blaa blaa");
                continue;
            }
            _qh.executeStatement("UPDATE tag SET isGroup='1' WHERE id=%s",
                                 QueryHelper::Bindings() << groupId);
            for (StringSet::const_iterator k = members.begin();
                 k != members.end(); ++k) {
                int memberId;
                try {
                    memberId = _qh.tagId(category, *k);
                }
                catch (EntryNotFoundError&) {
                    // TODO: this
                    qDebug("NYI: blee bloo blee");
                    continue;
                }
                _qh.executeStatement("INSERT INTO membergroup "
                                     "(groupTag, memberTag) VALUES (%s, %s)",
                                     QueryHelper::Bindings() <<
                                     groupId << memberId);
            }
        }
    }
}

#include "SQLMemberMap.moc"

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

#ifndef SQLMEMBERMAP_H
#define SQLMEMBERMAP_H

#include "DB/MemberMap.h"
#include "Connection.h"
#include "QueryHelper.h"

namespace SQLDB {

    class SQLMemberMap: public DB::MemberMap
    {
        Q_OBJECT

    public:
        /** Map from tag name to its members (set of tag names) within
         *  some category. */
        typedef QMap<QString, StringSet> CategoryGroups;

        /** Map from category name to its member group definitions. */
        typedef QMap<QString, CategoryGroups> MemberMapping;

        explicit SQLMemberMap(QueryHelper& queryHelper);
        DB::MemberMap& operator=(const DB::MemberMap& other);

        QStringList groups(const QString& category) const;
        void deleteGroup(const QString& category, const QString& name);
        QStringList members(const QString& category,
                            const QString& memberGroup,
                            bool closure) const;
        void setMembers(const QString& category,
                        const QString& memberGroup,
                        const QStringList& members);
        bool isEmpty() const;
        bool isGroup(const QString& category,
                     const QString& memberGroup) const;
        QMap<QString, StringSet> groupMap(const QString& category) const;
        QMap<QString, QStringList> inverseMap(const QString& category) const;
        void renameGroup(const QString& category,
                         const QString& oldName, const QString& newName);
        void renameCategory(const QString& oldName, const QString& newName);

        void addGroup(const QString& category, const QString& group);
        void addMemberToGroup(const QString& category,
                              const QString& group, const QString& item);
        void removeMemberFromGroup(const QString& category,
                                   const QString& group, const QString& item);

        const MemberMapping& memberMap() const;


    protected slots:
        void deleteItem(DB::Category* category, const QString& name);
        void renameItem(DB::Category* category,
                        const QString& oldName, const QString& newName);

    protected:
        QueryHelper& _qh;

    private:
        void overwriteWithMemberMap(const MemberMapping& map);
        void updateMemberMapCache() const;
        mutable MemberMapping _memberMapCache;

        // deny copy
        SQLMemberMap(const SQLMemberMap& other);
    };
}

#endif /* SQLMEMBERMAP_H */

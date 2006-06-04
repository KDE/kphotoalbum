/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef MEMBERMAP_H
#define MEMBERMAP_H
#include <qstringlist.h>
#include <qdom.h>
#include <qmap.h>
#include <qobject.h>

namespace XMLDB { class Database; }

namespace DB
{
class ImageDB;
class Category;

class MemberMap :public QObject {
    Q_OBJECT
public:
    MemberMap( ImageDB* db );
    MemberMap( const MemberMap& );
    MemberMap& operator=( const MemberMap& );

    QStringList groups( const QString& category ) const;
    void deleteGroup( const QString& category, const QString& name );
    QStringList members( const QString& category, const QString& memberGroup, bool closure ) const;
    void setMembers( const QString& category, const QString& memberGroup, const QStringList& members );
    bool isEmpty() const;
    bool isGroup( const QString& category, const QString& memberGroup ) const;
    QMap<QString,QStringList> groupMap( const QString& category );
    void renameGroup( const QString& category, const QString& oldName, const QString& newName );
    void renameCategory( const QString& oldName, const QString& newName );

    void addGroup( const QString& category, const QString& group );
    void addMemberToGroup( const QString& category, const QString& group, const QString& item );
    void removeMemberFromGroup( const QString& category, const QString& group, const QString& item );

protected:
    void calculate();
    QStringList calculateClosure( QMap<QString,QStringList>& resultSoFar, const QString& category, const QString& group );

protected slots:
    void deleteItem( Category* category, const QString& name);
    void renameItem( Category* category, const QString& oldName, const QString& newName );

private:
    friend class XMLDB::Database;
    // This is the primary data structure
    // { category |-> { group |-> [ member ] } } <- VDM syntax ;-)
    QMap<QString, QMap<QString,QStringList> > _members;

    // These are the data structures used to develop closures, they are only
    // needed to speed up the program *SIGNIFICANTLY* ;-)
    bool _dirty;
    QMap<QString, QMap<QString,QStringList> > _closureMembers;

};

}

#endif /* MEMBERMAP_H */


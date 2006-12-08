/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include <qmap.h>
#include <qobject.h>
#include "Utilities/Set.h"

namespace DB
{
class Category;

class MemberMap :public QObject {
    Q_OBJECT
public:
    MemberMap();
    MemberMap( const MemberMap& );
    virtual MemberMap& operator=( const MemberMap& );

    virtual QStringList groups( const QString& category ) const;
    virtual void deleteGroup( const QString& category, const QString& name );
    virtual QStringList members( const QString& category, const QString& memberGroup, bool closure ) const;
    virtual void setMembers( const QString& category, const QString& memberGroup, const QStringList& members );
    virtual bool isEmpty() const;
    virtual bool isGroup( const QString& category, const QString& memberGroup ) const;
    virtual QMap<QString,StringSet> groupMap( const QString& category ) const;
    virtual QMap<QString,QStringList> inverseMap( const QString& category ) const;
    virtual void renameGroup( const QString& category, const QString& oldName, const QString& newName );
    virtual void renameCategory( const QString& oldName, const QString& newName );

    virtual void addGroup( const QString& category, const QString& group );
    virtual void addMemberToGroup( const QString& category, const QString& group, const QString& item );
    virtual void removeMemberFromGroup( const QString& category, const QString& group, const QString& item );

    virtual const QMap<QString, QMap<QString,StringSet> >& memberMap() const { return _members; }

protected:
    void calculate() const;
    QStringList calculateClosure( QMap<QString,StringSet>& resultSoFar, const QString& category, const QString& group ) const;

public slots:
    virtual void deleteItem( DB::Category* category, const QString& name);
    virtual void renameItem( DB::Category* category, const QString& oldName, const QString& newName );
    void setLoading( bool b );

private:
    // This is the primary data structure
    // { category |-> { group |-> [ member ] } } <- VDM syntax ;-)
    QMap<QString, QMap<QString,StringSet> > _members;

    // These are the data structures used to develop closures, they are only
    // needed to speed up the program *SIGNIFICANTLY* ;-)
    mutable bool _dirty;
    mutable QMap<QString, QMap<QString,StringSet> > _closureMembers;

    bool _loading;
};

}

#endif /* MEMBERMAP_H */


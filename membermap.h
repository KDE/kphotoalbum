/* Copyright (C) 2003-2004 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef MEMBERMAP_H
#define MEMBERMAP_H
#include <qstringlist.h>
#include <qdom.h>
#include <qmap.h>
#include <qobject.h>

class MemberMap :public QObject {
    Q_OBJECT
public:
    MemberMap();
    MemberMap( const MemberMap& );
    MemberMap& operator=( const MemberMap& );

    QStringList groups( const QString& optionGroup ) const;
    void deleteGroup( const QString& optionGroup, const QString& name );
    QStringList members( const QString& optionGroup, const QString& memberGroup, bool closure ) const;
    void setMembers( const QString& optionGroup, const QString& memberGroup, const QStringList& members );
    QDomElement save( QDomDocument doc );
    bool isEmpty() const;
    void load( const QDomElement& );
    bool isGroup( const QString& optionGroup, const QString& memberGroup ) const;
    QMap<QString,QStringList> groupMap( const QString& optionGroup );
    void renameGroup( const QString& optionGroup, const QString& oldName, const QString& newName );

protected:
    void calculate();
    QStringList calculateClosure( QMap<QString,QStringList>& resultSoFar, const QString& optionGroup, const QString& group );

protected slots:
    void init();
    void deleteOption( const QString& optionGroup, const QString& name);
    void renameOption( const QString& optionGroup, const QString& oldName, const QString& newName );

private:
    // This is the primary data structure
    // { optionGroup |-> { group |-> [ member ] } } <- VDM syntax ;-)
    QMap<QString, QMap<QString,QStringList> > _members;

    // These are the data structures used to develop closures, they are only
    // needed to speed up the program *SIGNIFICANTLY* ;-)
    bool _dirty;
    QMap<QString, QMap<QString,QStringList> > _closureMembers;

};

#endif /* MEMBERMAP_H */


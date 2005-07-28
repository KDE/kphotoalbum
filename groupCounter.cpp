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

#include "groupCounter.h"
#include <qmap.h>
#include "options.h"
#include "membermap.h"
#include "imagedb.h"
#include "categorycollection.h"

// Examples:
// groupToMemberMap = { USA |-> [Chicago, Santa Clara],
//                      California |-> [Santa Clara, Los Angeles] }
// _memberToGroup = { Chicago |-> [USA],
//                    Sanata Clara |-> [ USA, California ],
//                    Los Angeless |-> [ California ] }

GroupCounter::GroupCounter( const QString& category )
{
    MemberMap map = ImageDB::instance()->memberMap();
    QMap<QString,QStringList> groupToMemberMap = map.groupMap(category);
    _memberToGroup.resize( 2729 /* A large prime */ );
    _groupCount.resize( 2729 /* A large prime */ );
    _memberToGroup.setAutoDelete( true );
    _groupCount.setAutoDelete( true );

    // Initialize _memberToGroup map.
    QStringList items = ImageDB::instance()->categoryCollection()->categoryForName( category )->items();
    items += map.groups( category );
    for( QStringList::Iterator it = items.begin(); it != items.end(); ++it ) {
        _memberToGroup.insert( *it, new QStringList );
    }

    // Populate the _memberToGroup map
    for( QMapIterator<QString,QStringList> it= groupToMemberMap.begin();
         it != groupToMemberMap.end(); ++it ) {
        QStringList list = it.data();
        QString group = it.key();

        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QStringList* item = _memberToGroup[*it2];
            Q_ASSERT( item );
            if ( item ) // better safe than sorry
                *item->append( group );
        }
        int* intPtr = new int;
        *intPtr = 0;
        _groupCount.insert( group, intPtr );
    }

}

/** optionList is the selected options for one image, members may be Las Vegas, Chicago, and Los Angeles if the
    category in question is Locations.
    This function then increases _groupCount with 1 for each of the groups the relavant items belongs to
    Las Vegas might increase the _groupCount[Nevada] by one.
    The tricky part is to avoid increasing it by more than 1 per image, that is what the countedGroupDict is
    used for.
*/
void GroupCounter::count( const QStringList& optionList )
{
    // It takes quite some time to clear the dict with a large prime!
    static QDict<void> countedGroupDict( 97 /* a large, but not extreme prime */ );

    countedGroupDict.clear();
    for( QStringList::ConstIterator optionIt = optionList.begin(); optionIt != optionList.end(); ++optionIt ) {
        QStringList* groups = _memberToGroup[*optionIt];
        if ( groups ) {
            for( QStringList::Iterator groupsIt = (*groups).begin(); groupsIt != (*groups).end(); ++groupsIt ) {
                if ( countedGroupDict.find( *groupsIt ) == 0 ) {
                    countedGroupDict.insert( *groupsIt, (void*) 0x1 ); // value not used, must be different from 0.
                    (*_groupCount[*groupsIt])++;
                }
            }
        }
        // The item Nevada should itself go into the group Nevada.
        if ( countedGroupDict.find( *optionIt ) == 0 && _groupCount.find( *optionIt ) ) {
             countedGroupDict.insert( *optionIt, (void*) 0x1 ); // value not used, must be different from 0.
             (*_groupCount[*optionIt])++;
        }
    }
}

QMap<QString,int> GroupCounter::result()
{
    QMap<QString,int> res;

    for( QDictIterator<int> it(_groupCount); *it; ++it ) {
        if ( *(*it) != 0 )
            res.insert( it.currentKey(), *(*it) );
    }
    return res;
}

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

#include "GroupCounter.h"
#include "DB/MemberMap.h"
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"

using namespace DB;


/**
 * \class DB::GroupCounter
 * \brief Utility class to help counting matches for member groups.
 *
 * This class is used to count the member group matches when
 * categorizing. The class is instantiating with the category we currently
 * are counting items for.
 *
 * The class builds the inverse member map, that is a map pointing from items
 * to parent.
 *
 * As an example, imagine we have the following member map (stored in the
 * variable groupToMemberMap in  the code):
 * \code
 *    { USA |-> [Chicago, Santa Clara],
 *      California |-> [Santa Clara, Los Angeles] }
 * \endcode
 *
 * The inverse map (stored in _memberToGroup in the code ) will then look
 * like this:
 * \code
 *  { Chicago |-> [USA],
 *    Sanata Clara |-> [ USA, California ],
 *    Los Angeless |-> [ California ] }
 * \endcode
 */
GroupCounter::GroupCounter( const QString& category )
{
    const MemberMap map = DB::ImageDB::instance()->memberMap();
    QMap<QString,StringSet> groupToMemberMap = map.groupMap(category);

    _memberToGroup.resize( 2729 /* A large prime */ );
    _groupCount.resize( 2729 /* A large prime */ );
    _memberToGroup.setAutoDelete( true );
    _groupCount.setAutoDelete( true );

    // Populate the _memberToGroup map
    for( QMapIterator<QString,StringSet> groupToMemberIt= groupToMemberMap.begin();
         groupToMemberIt != groupToMemberMap.end(); ++groupToMemberIt ) {

        StringSet members = groupToMemberIt.data();
        QString group = groupToMemberIt.key();

        for( StringSet::Iterator memberIt = members.begin(); memberIt != members.end(); ++memberIt ) {
            QStringList* item = _memberToGroup[*memberIt];
            if ( !item ) {
                item = new QStringList;
                _memberToGroup.insert(*memberIt, item );
            }

            *item->append( group );
        }
        uint* intPtr = new uint;
        *intPtr = 0;
        _groupCount.insert( group, intPtr );
    }

}

/**
 * categories is the selected categories for one image, members may be Las Vegas, Chicago, and Los Angeles if the
 * category in question is Places.
 * This function then increases _groupCount with 1 for each of the groups the relavant items belongs to
 * Las Vegas might increase the _groupCount[Nevada] by one.
 * The tricky part is to avoid increasing it by more than 1 per image, that is what the countedGroupDict is
 * used for.
 */
void GroupCounter::count( const StringSet& categories )
{
    // It takes quite some time to clear the dict with a large prime!
    static QDict<void> countedGroupDict( 97 /* a large, but not extreme prime */ );

    countedGroupDict.clear();
    for( StringSet::ConstIterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        QStringList* groups = _memberToGroup[*categoryIt];
        if ( groups ) {
            for( QStringList::Iterator groupsIt = (*groups).begin(); groupsIt != (*groups).end(); ++groupsIt ) {
                if ( countedGroupDict.find( *groupsIt ) == 0 ) {
                    countedGroupDict.insert( *groupsIt, (void*) 0x1 ); // value not used, must be different from 0.
                    (*_groupCount[*groupsIt])++;
                }
            }
        }
        // The item Nevada should itself go into the group Nevada.
        if ( countedGroupDict.find( *categoryIt ) == 0 && _groupCount.find( *categoryIt ) ) {
             countedGroupDict.insert( *categoryIt, (void*) 0x1 ); // value not used, must be different from 0.
             (*_groupCount[*categoryIt])++;
        }
    }
}

QMap<QString,uint> GroupCounter::result()
{
    QMap<QString,uint> res;

    for( QDictIterator<uint> it(_groupCount); *it; ++it ) {
        if ( *(*it) != 0 )
            res.insert( it.currentKey(), *(*it) );
    }
    return res;
}

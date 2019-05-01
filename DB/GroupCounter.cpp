/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "GroupCounter.h"
#include "DB/MemberMap.h"
#include "DB/ImageDB.h"
#include "Utilities/StringSet.h"
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
 * The inverse map (stored in m_memberToGroup in the code ) will then look
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

    m_memberToGroup.reserve( 2729 /* A large prime */ );
    m_groupCount.reserve( 2729 /* A large prime */ );

    // Populate the m_memberToGroup map
    for( QMap<QString,StringSet>::Iterator groupToMemberIt= groupToMemberMap.begin()
         ; groupToMemberIt != groupToMemberMap.end()
         ; ++groupToMemberIt )
    {
        StringSet members = groupToMemberIt.value();
        QString group = groupToMemberIt.key();

        Q_FOREACH( const auto &member, members ) {
            m_memberToGroup[member].append( group );
        }
        m_groupCount.insert( group, CountWithRange() );
    }
}

/**
 * categories is the selected categories for one image, members may be Las Vegas, Chicago, and Los Angeles if the
 * category in question is Places.
 * This function then increases m_groupCount with 1 for each of the groups the relavant items belongs to
 * Las Vegas might increase the m_groupCount[Nevada] by one.
 * The tricky part is to avoid increasing it by more than 1 per image, that is what the countedGroupDict is
 * used for.
 */
void GroupCounter::count( const StringSet& categories, const ImageDate &date )
{
    static StringSet countedGroupDict;

    countedGroupDict.clear();
    for( StringSet::const_iterator categoryIt = categories.begin(); categoryIt != categories.end(); ++categoryIt ) {
        if ( m_memberToGroup.contains(*categoryIt)) {
            const QStringList groups = m_memberToGroup[*categoryIt];
            for ( const QString& group : groups ) {
                if ( !countedGroupDict.contains( group ) ) {
                    countedGroupDict.insert( group );
                    m_groupCount[group].add(date);
                }
            }
        }
        // The item Nevada should itself go into the group Nevada.
        if ( !countedGroupDict.contains( *categoryIt ) && m_groupCount.contains( *categoryIt ) ) {
             countedGroupDict.insert( *categoryIt);
             m_groupCount[*categoryIt].add(date);
        }
    }
}

QMap<QString, CountWithRange> GroupCounter::result()
{
    QMap<QString,CountWithRange> res;

    for( QHash<QString,CountWithRange>::const_iterator it = m_groupCount.constBegin(); it != m_groupCount.constEnd(); ++it) {
        if ( it.value().count != 0 )
            res.insert( it.key(), it.value() );
    }
    return res;
}
// vi:expandtab:tabstop=4 shiftwidth=4:

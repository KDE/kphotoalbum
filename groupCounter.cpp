#include "groupCounter.h"
#include <qmap.h>
#include "options.h"

GroupCounter::GroupCounter( const QString& optionGroup )
{
    MemberMap map = Options::instance()->memberMap();
    QMap<QString,QStringList> groupToMemberMap = map.groupMap(optionGroup);
    _memberToGroup.resize( 2729 /* A large prime */ );
    _groupCount.resize( 2729 /* A large prime */ );

    // Initialize _memberToGroup map.
    QStringList items = Options::instance()->optionValue( optionGroup );
    items += map.groups( optionGroup );
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

/* optionList is the selected options for one image, members may be Las Vegas, Chicago, and Los Angeles if the
   option group in question is Locations.
   This function then increases _groupCount with 1 for each of the groups the relavant items belongs to
   Las Vegas might increase the _groupCount[Nevada] by one.
   The tricky part is to avoid increasing it by more than 1 per image, that is what the countedGroupDict is
   used for.
*/
void GroupCounter::count( const QStringList& optionList )
{
    QDict<void> countedGroupDict( 2729 /* a large prime */ );
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

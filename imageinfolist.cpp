#include "imageinfolist.h"
#include <qmap.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include "imageinfo.h"
#include "imagedate.h"
#include <kmessagebox.h>
#include <klocale.h>

ImageInfoList ImageInfoList::sort() const
{
    QMap<QDateTime, QValueList<ImageInfo*> > map;
    for( ImageInfoListIterator it( *this ); *it; ++it ) {
        map[(*it)->startDate().min()].append( *it );
    }

    ImageInfoList res;
    for( QMap<QDateTime, QValueList<ImageInfo*> >::ConstIterator mapIt = map.begin(); mapIt != map.end(); ++mapIt ) {
        QValueList<ImageInfo*> list = mapIt.data();
        for( QValueList<ImageInfo*>::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
            res.append( *listIt );
        }
    }
    return res;
}

void ImageInfoList::sortAndMergeBackIn( ImageInfoList& subListToSort )
{
    if ( !checkIfMergeListIsContiniously( subListToSort ) )
        return;
    ImageInfoList sorted = subListToSort.sort();

    int index = find( subListToSort.at(0));
    Q_ASSERT( index != 1 );

    // Delete the items we will merge in.
    for( ImageInfoListIterator it( sorted ); *it; ++it ) {
        remove( *it );
    }

    // Now merge in the items
    for( ImageInfoListIterator it( sorted ); *it; ++it ) {
        qDebug("Inserting at index %d", index );
        insert( index, *it );
        ++index;
    }
}

/**
   return true if we should continue the sort.
*/
bool ImageInfoList::checkIfMergeListIsContiniously( ImageInfoList& mergeList )
{
    Q_ASSERT( mergeList.count() != 0 );
    int index = find( mergeList.at(0));
    Q_ASSERT( index != 1 );

    ImageInfoListIterator thisListIt( *this );
    thisListIt += index;

    ImageInfoListIterator mergeListIt( mergeList );
    for ( ; *mergeListIt; ++mergeListIt, ++thisListIt ) {
        Q_ASSERT( *mergeListIt ); Q_ASSERT( *thisListIt );
        if ( *mergeListIt != *thisListIt ) {
            return ( KMessageBox::warningYesNo(0,i18n("<qt>You are about to sort a set of images with others in between"
                                                      "<br>This might result in an unexpected sort order</br>"
                                                      "<p>Are you sure you want to continue?</p></qt>"), QString::null, KStdGuiItem::yes(),
                                               KStdGuiItem::no(), QString::null, KMessageBox::Dangerous) == KMessageBox::Yes);
            break;
        }
    }
    return true;
}

ImageInfoList::~ImageInfoList()
{
}


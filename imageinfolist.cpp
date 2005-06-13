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
    QMap<QDateTime, QValueList<ImageInfoPtr> > map;
    for( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        map[(*it)->startDate().min()].append( *it );
    }

    ImageInfoList res;
    for( QMap<QDateTime, QValueList<ImageInfoPtr> >::ConstIterator mapIt = map.begin(); mapIt != map.end(); ++mapIt ) {
        QValueList<ImageInfoPtr> list = mapIt.data();
        for( QValueList<ImageInfoPtr>::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
            res.append( *listIt );
        }
    }
    return res;
}

void ImageInfoList::sortAndMergeBackIn( ImageInfoList& /*subListToSort*/ )
{
    // qDebug( "REM: ImageInfoList::sortAndMergeBackIn" );
#ifdef TEMPORARILY_REMOVED
    qDebug("Temp removed ImageInfoList::sortAndMergeBackIn" );
#ifdef TEMPORARILY_REMOVED
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
        insert( index, *it );
        ++index;
    }
#endif
#endif
}

/**
   return true if we should continue the sort.
*/
bool ImageInfoList::checkIfMergeListIsContiniously( ImageInfoList& /*mergeList*/ )
{
    qDebug( "Temp removed ImageInfoList::checkIfMergeListIsContiniously" );
#ifdef TEMPORARILY_REMOVED
    Q_ASSERT( mergeList.count() != 0 );
    int index = find( mergeList.at(0));

    ImageInfoListIterator thisListIt( *this );
    thisListIt += index;

    ImageInfoListIterator mergeListIt( mergeList );
    for ( ; *mergeListIt; ++mergeListIt, ++thisListIt ) {
        Q_ASSERT( *mergeListIt ); Q_ASSERT( *thisListIt );
        if ( *mergeListIt != *thisListIt ) {
            return ( KMessageBox::warningContinueCancel(0,i18n("<qt>You are about to sort a set of images with others in between"
                                                      "<br>This might result in an unexpected sort order</br>"
                                                      "<p>Are you sure you want to continue?</p></qt>"), i18n("Sort Images?") ) == KMessageBox::Continue);
            break;
        }
    }
    return true;
#endif
    return true;
}

ImageInfoList::~ImageInfoList()
{
}

void ImageInfoList::appendList( ImageInfoList& list )
{
    for ( ImageInfoListConstIterator it = list.constBegin(); it != list.constEnd(); ++it ) {
        append( *it );
    }
}

void ImageInfoList::printItems()
{
    for ( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        qDebug("%s", (*it)->fileName().latin1() );
    }
}

bool ImageInfoList::isSorted()
{
    if ( count() == 0 )
        return true;

    QDateTime prev = first()->startDate().min();
    for ( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        QDateTime cur = (*it)->startDate().min();
        if ( prev > cur )
            return false;
        prev = cur;
    }
    return true;
}

void ImageInfoList::mergeIn( ImageInfoList /*other*/)
{
    qDebug("Temp removed ImageInfoList::mergeIn" );
#ifdef TEMPORARILY_REMOVED
    ImageInfoList tmp;

    for ( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        QDateTime thisDate = (*it)->startDate().min();
        while ( other.count() != 0 ) {
            QDateTime otherDate = other.first()->startDate().min();
            if ( otherDate < thisDate )
                tmp.append( other.take(0) );
            else
                break;
        }
        tmp.append( *it );
    }
    tmp.appendList( other );
    *this = tmp;
#endif
}

void ImageInfoList::remove( ImageInfoPtr info )
{
    for( ImageInfoListIterator it = begin(); it != end(); ++it ) {
        if ( (*(*it)) == *info ) {
            qDebug("Found it!");
            QValueList<ImageInfoPtr>::remove(it);
            return;
        }
    }
}


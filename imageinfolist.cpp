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
        map[(*it)->date().start()].append( *it );
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

void ImageInfoList::sortAndMergeBackIn( ImageInfoList& subListToSort )
{
    if ( !checkIfMergeListIsContiniously( subListToSort ) )
        return;
    ImageInfoList sorted = subListToSort.sort();

    ImageInfoListIterator insertIt = find( subListToSort[0]);
    --insertIt;

    // Delete the items we will merge in.
    for( ImageInfoListIterator it = sorted.begin(); it != sorted.end(); ++it ) {
        remove( *it );
    }

    ++insertIt;

    // Now merge in the items
    for( ImageInfoListIterator it = sorted.begin(); it != sorted.end(); ++it )
        insert( insertIt, *it );
}

/**
   return true if we should continue the sort.
*/
bool ImageInfoList::checkIfMergeListIsContiniously( ImageInfoList& mergeList )
{
    Q_ASSERT( mergeList.count() != 0 );
    ImageInfoListIterator thisListIt = find( mergeList[0]);

    for( ImageInfoListIterator mergeListIt = mergeList.begin(); mergeListIt != mergeList.end(); ++mergeListIt, ++thisListIt ) {
        Q_ASSERT( *mergeListIt ); Q_ASSERT( *thisListIt );
        if ( *mergeListIt != *thisListIt ) {

            return ( KMessageBox::warningContinueCancel(0,i18n("<qt>You are about to sort a set of images with others in between"
                                                      "<br>This might result in an unexpected sort order</br>"
                                                      "<p>Are you sure you want to continue?</p></qt>"), i18n("Sort Images?") ) == KMessageBox::Continue);
            break;
        }
    }
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

    QDateTime prev = first()->date().start();
    for ( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        QDateTime cur = (*it)->date().start();
        if ( prev > cur )
            return false;
        prev = cur;
    }
    return true;
}

void ImageInfoList::mergeIn( ImageInfoList other)
{
    ImageInfoList tmp;

    for ( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        QDateTime thisDate = (*it)->date().start();
        while ( other.count() != 0 ) {
            QDateTime otherDate = other.first()->date().start();
            if ( otherDate < thisDate ) {
                tmp.append( other[0] );
                other.pop_front();
            }
            else
                break;
        }
        tmp.append( *it );
    }
    tmp.appendList( other );
    *this = tmp;
}

void ImageInfoList::remove( ImageInfoPtr info )
{
    for( ImageInfoListIterator it = begin(); it != end(); ++it ) {
        if ( (*(*it)) == *info ) {
            QValueList<ImageInfoPtr>::remove(it);
            return;
        }
    }
}

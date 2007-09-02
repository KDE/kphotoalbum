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
#include "ImageInfoList.h"
#include <qmap.h>
#include <qdatetime.h>
#include <qvaluelist.h>
#include "ImageInfo.h"
#include <kmessagebox.h>
#include <klocale.h>

using namespace DB;

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

            return ( KMessageBox::warningContinueCancel(0,i18n("<p>You are about to sort a set of thumbnails with others in between.<br />"
                                                      "This might result in an unexpected sort order.<br />"
                                                      "<p>Are you sure you want to continue?</p>"), i18n("Sort Thumbnails?") ) == KMessageBox::Continue);
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

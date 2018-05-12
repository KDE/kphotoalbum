/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
#include "ImageInfoList.h"
#include "ImageInfo.h"
#include "Logging.h"

#include <DB/FileNameList.h>

#include <QVector>
#include <QtAlgorithms>
#include <QDateTime>

#include <KMessageBox>
#include <KLocalizedString>
using namespace DB;

class SortableImageInfo
{
public:
    SortableImageInfo(const QDateTime& datetime, const QString& string, const ImageInfoPtr &info)
        : m_dt(datetime), m_st(string), m_in(info) {}
    SortableImageInfo(const SortableImageInfo& in)
        : m_dt(in.m_dt), m_st(in.m_st), m_in(in.m_in) {}
    SortableImageInfo() {}
    ~SortableImageInfo() {}
    const QDateTime& DateTime(void) const { return m_dt; }
    const QString& String(void) const { return m_st; }
    const ImageInfoPtr& ImageInfo(void) const { return m_in; }
    bool operator== (const SortableImageInfo& other) const { return m_dt == other.m_dt && m_st == other.m_st; }
    bool operator!= (const SortableImageInfo& other) const { return m_dt != other.m_dt || m_st != other.m_st; }
    bool operator> (const SortableImageInfo& other) const { if (m_dt != other.m_dt) { return m_dt > other.m_dt; } else { return m_st > other.m_st; }}
    bool operator< (const SortableImageInfo& other) const { if (m_dt != other.m_dt) { return m_dt < other.m_dt; } else { return m_st < other.m_st; }}
    bool operator>= (const SortableImageInfo& other) const { return *this == other || *this > other; }
    bool operator<= (const SortableImageInfo& other) const { return *this == other || *this < other; }

private:
    QDateTime m_dt;
    QString m_st;
    ImageInfoPtr m_in;
};

ImageInfoList ImageInfoList::sort() const
{
    QVector<SortableImageInfo> vec;
    for( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        vec.append(SortableImageInfo((*it)->date().start(),(*it)->fileName().absolute(), *it));
    }

    std::sort(vec.begin(),vec.end());
    
    ImageInfoList res;
    for( QVector<SortableImageInfo>::ConstIterator mapIt = vec.constBegin(); mapIt != vec.constEnd(); ++mapIt ) {
         res.append(mapIt->ImageInfo());
    }
    return res;
}

void ImageInfoList::sortAndMergeBackIn( ImageInfoList& subListToSort )
{
    ImageInfoList sorted = subListToSort.sort();

    const int insertIndex = indexOf(subListToSort[0]);
    Q_ASSERT(insertIndex >= 0);

    // Delete the items we will merge in.
    for( ImageInfoListIterator it = sorted.begin(); it != sorted.end(); ++it )
        remove( *it );

    ImageInfoListIterator insertIt = begin() + insertIndex;

    // Now merge in the items
    for( ImageInfoListIterator it = sorted.begin(); it != sorted.end(); ++it ) {
        insertIt = insert( insertIt, *it );
        ++insertIt;
    }
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
        qCDebug(DBLog) << (*it)->fileName().absolute();
    }
}

bool ImageInfoList::isSorted()
{
    if ( count() == 0 )
        return true;

    QDateTime prev = first()->date().start();
    QString prevFile = first()->fileName().absolute();
    for ( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        QDateTime cur = (*it)->date().start();
        QString curFile = (*it)->fileName().absolute();
        if ( prev > cur ||
             ( prev == cur && prevFile > curFile ) )
            return false;
        prev = cur;
        prevFile = curFile;
    }
    return true;
}

void ImageInfoList::mergeIn( ImageInfoList other)
{
    ImageInfoList tmp;

    for ( ImageInfoListConstIterator it = constBegin(); it != constEnd(); ++it ) {
        QDateTime thisDate = (*it)->date().start();
        QString thisFileName = (*it)->fileName().absolute();
        while ( other.count() != 0 ) {
            QDateTime otherDate = other.first()->date().start();
            QString otherFileName = other.first()->fileName().absolute();
            if ( otherDate < thisDate ||
                 ( otherDate == thisDate && otherFileName < thisFileName ) ) {
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

void ImageInfoList::remove( const ImageInfoPtr& info )
{
    for( ImageInfoListIterator it = begin(); it != end(); ++it ) {
        if ( (*(*it)) == *info ) {
            QList<ImageInfoPtr>::erase(it);
            return;
        }
    }
}

DB::FileNameList ImageInfoList::files() const
{
    DB::FileNameList res;
    for ( const ImageInfoPtr& info : *this)
        res.append(info->fileName());
    return res;
}
// vi:expandtab:tabstop=4 shiftwidth=4:

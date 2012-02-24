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
#ifndef DB_RESULT_H
#define DB_RESULT_H

#include "RawId.h"
#include "ImageInfoPtr.h"

namespace DB
{
class Id;

/** List of media item ids.
 *
 * Instances are implicitly shared, so they can be passed by value in
 * constant time.
 *
 * This class implements forward iterator API and it is possible to
 * use Q_FOREACH to iterate over the Id objects in the list.
 *
 * Iterating example:
 * \code
 * DB::IdList list = getSomeIdList();
 * Q_FOREACH(DB::Id id, list) {
 *     doSomethingWithMediaId(id);
 * }
 * \endcode
 */
class IdList
{
 public:
    class ConstIterator {
    public:
        ConstIterator& operator++();
        DB::Id operator*();
        bool operator==( const ConstIterator& other ) const;
        bool operator!=( const ConstIterator& other ) const;

    private:
        friend class IdList;
        ConstIterator( const IdList* result, int pos );

        const IdList* _result;
        int _pos;
    };
    typedef ConstIterator const_iterator;

    IdList();

    /** Create a result with a list of raw ids. */
    explicit IdList( const QList<DB::RawId>& ids );

    /** Convenience constructor: create a result only one Id. */
    explicit IdList( const DB::Id& );

    void append( const DB::Id& );
    void prepend( const DB::Id& );
    void removeAll(const DB::Id&);
    IdList reversed() const;

    DB::Id at(int index) const;
    int size() const;
    bool isEmpty() const;
    int indexOf(const DB::Id&) const;
    ConstIterator begin() const;
    ConstIterator end() const;
    void debug() const;

    QList<DB::ImageInfoPtr> fetchInfos() const;

    /** Get the raw list for offline manipulation */
    const QList<DB::RawId>& rawIdList() const;

 private:
    QList<DB::RawId> _items;
};

}  // namespace DB


#endif /* DB_RESULT_H */


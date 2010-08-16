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
class ResultId;

/** List of media item ids.
 *
 * Instances are implicitly shared, so they can be passed by value in
 * constant time.
 *
 * This class implements forward iterator API and it is possible to
 * use Q_FOREACH to iterate over the ResultId objects in the list.
 *
 * Iterating example:
 * \code
 * DB::Result list = getSomeIdList();
 * Q_FOREACH(DB::ResultId id, list) {
 *     doSomethingWithMediaId(id);
 * }
 * \endcode
 *
 * \todo Rename to IdList as per discussion in car
 */
class Result
{
 public:
    class ConstIterator {
    public:
        ConstIterator& operator++();
        DB::ResultId operator*();
        bool operator==( const ConstIterator& other ) const;
        bool operator!=( const ConstIterator& other ) const;

    private:
        friend class Result;
        ConstIterator( const Result* result, int pos );

        const Result* _result;
        int _pos;
    };
    typedef ConstIterator const_iterator;

    Result();

    /** Create a result with a list of raw ids. */
    explicit Result( const QList<DB::RawId>& ids );

    /** Convenience constructor: create a result only one ResultId. */
    explicit Result( const DB::ResultId& );

    void append( const DB::ResultId& );
    void prepend( const DB::ResultId& );
    void removeAll(const DB::ResultId&);
    Result reversed() const;

    DB::ResultId at(int index) const;
    int size() const;
    bool isEmpty() const;
    int indexOf(const DB::ResultId&) const;
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


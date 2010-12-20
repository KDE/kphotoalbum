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
#include "Result.h"
#include "ResultId.h"
#include "ImageInfo.h"

DB::Result::ConstIterator::ConstIterator( const Result* result, int pos )
    :_result(result), _pos(pos)
{
}

DB::Result::ConstIterator& DB::Result::ConstIterator::operator++()
{
    ++_pos;
    return *this;
}

DB::ResultId DB::Result::ConstIterator::operator*()
{
    return _result->at(_pos);
}

DB::Result::ConstIterator DB::Result::begin() const
{
    return DB::Result::ConstIterator(this, 0);
}

DB::Result::ConstIterator DB::Result::end() const
{
    return DB::Result::ConstIterator( this, size() );
}

bool DB::Result::ConstIterator::operator==( const ConstIterator& other ) const
{
    return _pos == other._pos;
}

bool DB::Result::ConstIterator::operator!=( const ConstIterator& other ) const
{
    return _pos != other._pos;
}

DB::ResultId DB::Result::at(int index) const
{
    return DB::ResultId(_items[index], *this);
}

uint DB::Result::size() const
{
    return _items.size();
}

int DB::Result::indexOf(const DB::ResultId& id) const
{
    return _items.indexOf(id.rawId());
}

DB::Result::Result(const QList<DB::RawId>& ids)
    :_items(ids)
{
}

DB::Result::Result( const DB::ResultId& id) {
    _items.push_back(id.rawId());
}

DB::Result::Result()
{
}

void DB::Result::debug() const
{
    qDebug() << "Size: " << size();
    qDebug() << _items;
}

void DB::Result::append( const DB::ResultId& id)
{
    _items.append(id.rawId());
}

void DB::Result::prepend( const DB::ResultId& id)
{
    _items.prepend(id.rawId());
}

void DB::Result::removeAll(const DB::ResultId& id)
{
    _items.removeAll(id.rawId());
}

bool DB::Result::isEmpty() const
{
    return _items.isEmpty();
}

QList<DB::ImageInfoPtr> DB::Result::fetchInfos() const
{
    QList<DB::ImageInfoPtr> infos;
    for (const_iterator i = begin(); i != end(); ++i)
        infos.push_back((*i).fetchInfo());
    return infos;
}

const QList<DB::RawId>& DB::Result::rawIdList() const
{
    return _items;
}

DB::Result DB::Result::reversed() const
{
    Result res;
    Q_FOREACH(ResultId id, *this) {
        res.prepend(id);
    }
    return res;
}

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
#include "IdList.h"
#include "Id.h"
#include "ImageInfo.h"

DB::IdList::ConstIterator::ConstIterator( const IdList* result, int pos )
    :_result(result), _pos(pos)
{
}

DB::IdList::ConstIterator& DB::IdList::ConstIterator::operator++()
{
    ++_pos;
    return *this;
}

DB::Id DB::IdList::ConstIterator::operator*()
{
    return _result->at(_pos);
}

DB::IdList::ConstIterator DB::IdList::begin() const
{
    return DB::IdList::ConstIterator(this, 0);
}

DB::IdList::ConstIterator DB::IdList::end() const
{
    return DB::IdList::ConstIterator( this, size() );
}

bool DB::IdList::ConstIterator::operator==( const ConstIterator& other ) const
{
    return _pos == other._pos;
}

bool DB::IdList::ConstIterator::operator!=( const ConstIterator& other ) const
{
    return _pos != other._pos;
}

DB::Id DB::IdList::at(int index) const
{
    return DB::Id(_items[index], *this);
}

int DB::IdList::size() const
{
    return _items.size();
}

int DB::IdList::indexOf(const DB::Id& id) const
{
    return _items.indexOf(id.rawId());
}

DB::IdList::IdList(const QList<DB::RawId>& ids)
    :_items(ids)
{
}

DB::IdList::IdList( const DB::Id& id) {
    _items.push_back(id.rawId());
}

DB::IdList::IdList()
{
}

void DB::IdList::debug() const
{
    qDebug() << "Size: " << size();
    qDebug() << _items;
}

void DB::IdList::append( const DB::Id& id)
{
    _items.append(id.rawId());
}

void DB::IdList::prepend( const DB::Id& id)
{
    _items.prepend(id.rawId());
}

void DB::IdList::removeAll(const DB::Id& id)
{
    _items.removeAll(id.rawId());
}

bool DB::IdList::isEmpty() const
{
    return _items.isEmpty();
}

QList<DB::ImageInfoPtr> DB::IdList::fetchInfos() const
{
    QList<DB::ImageInfoPtr> infos;
    for (const_iterator i = begin(); i != end(); ++i)
        infos.push_back((*i).fetchInfo());
    return infos;
}

const QList<DB::RawId>& DB::IdList::rawIdList() const
{
    return _items;
}

DB::IdList DB::IdList::reversed() const
{
    IdList res;
    Q_FOREACH(const Id& id, *this) {
        res.prepend(id);
    }
    return res;
}

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
#include "Id.h"
#include "IdList.h"
#include "ImageDB.h"

DB::Id const DB::Id::null;

DB::Id::Id()
    : _rawId()
    , _context()
{
    Q_ASSERT(isNull());
}

DB::Id::Id(DB::RawId rawId, const IdList& context)
    : _rawId(rawId)
    , _context(context)
{
    Q_ASSERT(!isNull());
    Q_ASSERT(!_context.isEmpty());
}

DB::RawId DB::Id::rawId() const
{
    return _rawId;
}

bool DB::Id::isNull() const {
    Q_ASSERT(_rawId == DB::RawId() || toInt(_rawId) >= 1);
    return _rawId == DB::RawId();
}

DB::ImageInfoPtr DB::Id::fetchInfo() const {
    if (isNull()) return ImageInfoPtr(NULL);
    return DB::ImageDB::instance()->info(*this);
}

const DB::IdList& DB::Id::context() const
{
    return _context;
}


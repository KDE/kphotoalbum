/*
  Copyright (C) 2008-2010 Tuomas Suutari <thsuut@utu.fi>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING); if not, write to the
  Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
  MA 02110-1301 USA.
*/

#ifndef DB_RAWID_H
#define DB_RAWID_H

#include <QVariant>
#include <QDebug>

#ifndef DB_RAWID_IS_PLAIN_INTEGER

namespace DB {
class RawId;
}

inline int toInt(const DB::RawId rawId);

inline unsigned int qHash(const DB::RawId rawId);

namespace DB {

class RawId
{
    friend inline int ::toInt(const DB::RawId rawId);
    friend inline unsigned int ::qHash(const DB::RawId rawId);

public:
    RawId()
        : _value(nullValue)
    {
    }

    explicit RawId(int value)
        : _value(value)
    {
        Q_ASSERT(_value != nullValue);
        Q_ASSERT(_value > 0);
    }

    bool operator==(const RawId other) const
    {
        return _value == other._value;
    }

    bool operator!=(const RawId other) const
    {
        return _value != other._value;
    }

    bool operator<(const RawId other) const
    {
        return _value < other._value;
    }

    operator QVariant() const
    {
        return QVariant(_value);
    }

private:
    static const int nullValue = -1;

    int _value;
};

} // end of namespace DB

inline int toInt(const DB::RawId rawId)
{
    Q_ASSERT(rawId != DB::RawId());
    return rawId._value;
}

inline unsigned int qHash(const DB::RawId rawId)
{
    return rawId._value;
}

inline QDebug operator<<(QDebug d, const DB::RawId rawId)
{
    return (d << toInt(rawId));
}

#else

namespace DB {

typedef int RawId;

} // end of namespace DB

inline int toInt(const DB::RawId rawId)
{
    return rawId;
}

#endif

#endif // DB_RAWID_H
// vi:expandtab:tabstop=4 shiftwidth=4:

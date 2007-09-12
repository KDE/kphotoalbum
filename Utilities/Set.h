/*
  Copyright (C) 2007 Tuomas Suutari <thsuut@utu.fi>
  Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef UTILITIES_SET_H
#define UTILITIES_SET_H

#include <qvaluelist.h>
#include <qdatastream.h>
#include <set>

namespace Utilities
{
    template <class T>
    class Set: public std::set<T>
    {
        typedef std::set<T> Base;

    public:
        Set() {}
        Set(const QValueList<T>& list)
        {
            insert(list);
        }

        void insert(const T& x)
        {
            Base::insert(x);
        }

        void insert(const QValueList<T>& list);

        QValueList<T> toList() const;

        bool contains(const T& x) const
        {
            return count(x) > 0;
        }

        Set<T>& operator+=(const Set<T>& other);

        Set<T> operator+(const Set<T>& other) const
        {
            return (Set<T>(*this) += other);
        }

        Set<T>& operator-=(const Set<T>& other);

        Set<T> operator-(const Set<T>& other) const
        {
            return (Set<T>(*this) -= other);
        }
    };

    typedef Set<QString> StringSet;
}

template <class TYPE>
QDataStream& operator<<(QDataStream& stream, const Utilities::Set<TYPE>& data);

template <class TYPE>
QDataStream& operator>>(QDataStream& stream, Utilities::Set<TYPE>& data);

#endif /* UTILITIES_SET_H */

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

#ifndef SET_H
#define SET_H

#include <QMap>
#include <QList>
#include <QDataStream>

namespace Utilities
{
    template <class T>
    class Set: public QMap<T, T>
    {
    public:
        Set() {}
        Set(const QList<T>& list);

        void insert(T key);

        void insert(const QList<T>& list);

        QList<T> toList() const;

        bool operator==(const Set<T>& other) const;

        bool operator!=(const Set<T>& other) const
        {
            return !(*this == other);
        }

        Set<T>& operator+=(const Set<T>& other);

        Set<T> operator+(const Set<T>& other) const;

        Set<T>& operator-=(const Set<T>& other);

        Set<T> operator-(const Set<T>& other) const;
    };

    typedef Set<QString> StringSet;
}

template <class TYPE>
QDataStream& operator<<(QDataStream& stream, const Utilities::Set<TYPE>& data);

template <class TYPE>
QDataStream& operator>>(QDataStream& stream, Utilities::Set<TYPE>& data);

#endif /* SET_H */

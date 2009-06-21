/*
  Copyright (C) 2006-2007 Tuomas Suutari <thsuut@utu.fi>

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

#include "List.h"
#include "DB/RawId.h"
#include <QTime>
#include <QList>
#include <stdlib.h> // rand
#include <algorithm> // std::swap
#include <QStringList>

template <class T>
QList<T> Utilities::mergeListsUniqly(const QList<T>& l1, const QList<T>& l2)
{
    QList<T> r = l1;
    Q_FOREACH(const T& x, l2)
        if (!r.contains(x))
            r.append(x);
    return r;
}

template <class T>
QList<T> Utilities::listSubtract(const QList<T>& l1, const QList<T>& l2)
{
    QList<T> r = l1;
    Q_FOREACH(const T& x, l2)
        r.removeAll(x);
    return r;
}

namespace
{
    template <class T>
    class AutoDeletedArray
    {
    public:
        AutoDeletedArray(uint size): _ptr(new T[size]) {}
        operator T*() const { return _ptr; }
        ~AutoDeletedArray() { delete[] _ptr; }
    private:
        T* _ptr;
    };
}

template <class T>
QList<T> Utilities::shuffleList(const QList<T>& list)
{
    static bool init = false;
    if ( !init ) {
        QTime midnight( 0, 0, 0 );
        srand( midnight.secsTo(QTime::currentTime()) );
        init = true;
    }

    // Take pointers from input list to an array for shuffling
    uint N = list.size();
    AutoDeletedArray<const T*> deck(N);
    const T** p = deck;
    for (typename QList<T>::const_iterator i = list.begin();
         i != list.end(); ++i) {
        *p = &(*i);
        ++p;
    }

    // Shuffle the array of pointers
    for (uint i = 0; i < N; i++) {
        uint r = i + static_cast<uint>(static_cast<double>(N - i) * rand() /
                                       static_cast<double>(RAND_MAX));
        std::swap(deck[r], deck[i]);
    }

    // Create new list from the array
    QList<T> result;
    const T** const onePastLast = deck + N;
    for (p = deck; p != onePastLast; ++p)
        result.push_back(**p);

    return result;
}

template <class T>
QList<QVariant> Utilities::toVariantList(const T& l)
{
    QList<QVariant> r;
    for (typename T::const_iterator i = l.begin(); i != l.end(); ++i)
        r << *i;
    return r;
}

#define INSTANTIATE_MERGELISTSUNIQLY(T) \
template \
QList<T> Utilities::mergeListsUniqly(const QList<T>& l1, const QList<T>& l2)

#define INSTANTIATE_LISTSUBTRACT(T) \
template \
QList<T> Utilities::listSubtract(const QList<T>& l1, const QList<T>& l2)

#define INSTANTIATE_SHUFFLELIST(T) \
template \
QList<T> Utilities::shuffleList(const QList<T>& list)

#define INSTANTIATE_TOVARIANTLIST(T) \
template \
QList<QVariant> Utilities::toVariantList(const T& l)

INSTANTIATE_MERGELISTSUNIQLY(DB::RawId);
INSTANTIATE_MERGELISTSUNIQLY(QString);
INSTANTIATE_LISTSUBTRACT(DB::RawId);
INSTANTIATE_SHUFFLELIST(DB::RawId);
INSTANTIATE_TOVARIANTLIST(QList<int>);
#ifndef DB_RAWID_IS_PLAIN_INTEGER
INSTANTIATE_TOVARIANTLIST(QList<DB::RawId>);
#endif
INSTANTIATE_TOVARIANTLIST(QStringList);

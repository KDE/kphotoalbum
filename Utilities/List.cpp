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
#include <qstring.h>
#include <qdatetime.h>
#include <stdlib.h> // rand
#include <algorithm> // std::swap

template <class T>
QValueList<T> Utilities::mergeListsUniqly(const QValueList<T>& l1,
                                          const QValueList<T>& l2)
{
    QValueList<T> r = l1;
    for (typename QValueList<T>::const_iterator i = l2.begin();
         i != l2.end(); ++i)
        if (!r.contains(*i))
            r.append(*i);
    return r;
}

template <class T>
QValueList<T> Utilities::listSubtract(const QValueList<T>& l1,
                                      const QValueList<T>& l2)
{
    QValueList<T> r = l1;
    for (typename QValueList<T>::const_iterator i = l2.begin();
         i != l2.end(); ++i)
        r.remove(*i);
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
QValueList<T> Utilities::shuffleList(const QValueList<T>& list)
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
    for (typename QValueList<T>::const_iterator i = list.begin();
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
    QValueList<T> result;
    const T** const onePastLast = deck + N;
    for (p = deck; p != onePastLast; ++p)
        result.push_back(**p);

    return result;
}

template <class T>
QValueList<QVariant> Utilities::toVariantList(const T& l)
{
    QValueList<QVariant> r;
    for (typename T::const_iterator i = l.begin(); i != l.end(); ++i)
        r << *i;
    return r;
}


#define INSTANTIATE_MERGELISTSUNIQLY(T) \
template \
QValueList<T> Utilities::mergeListsUniqly(const QValueList<T>& l1, \
                                          const QValueList<T>& l2)

#define INSTANTIATE_LISTSUBTRACT(T) \
template \
QValueList<T> Utilities::listSubtract(const QValueList<T>& l1, \
                                      const QValueList<T>& l2)

#define INSTANTIATE_SHUFFLELIST(T) \
template \
QValueList<T> Utilities::shuffleList(const QValueList<T>& list)

#define INSTANTIATE_TOVARIANTLIST(T) \
template \
QValueList<QVariant> Utilities::toVariantList(const T& l)

INSTANTIATE_MERGELISTSUNIQLY(int);
INSTANTIATE_MERGELISTSUNIQLY(QString);
INSTANTIATE_LISTSUBTRACT(int);
INSTANTIATE_SHUFFLELIST(QString);
INSTANTIATE_TOVARIANTLIST(QValueList<int>);
INSTANTIATE_TOVARIANTLIST(QStringList);

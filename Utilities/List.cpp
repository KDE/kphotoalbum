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
#include "Set.h"
#include <QString>
#include <QTime>
#include <stdlib.h> // rand
#include <algorithm> // std::swap


// These are very simple and unoptimal implementations.
// (Mainly because searching linked list is O(n).)
// Could be optimized later, but no need for that yet.

template <class T>
QList<T> Utilities::mergeListsUniqly(const QList<T>& l1,
                                     const QList<T>& l2)
{
    QList<T> r;
    const QList<T>* l[2] = {&l1, &l2};
    for (int n = 0; n < 2; ++n)
        for (typename QList<T>::const_iterator i = l[n]->begin();
             i != l[n]->end(); ++i)
            if (!r.contains(*i))
                r.append(*i);
    return r;
}

template <class T>
QList<T> Utilities::listSubtract(const QList<T>& l1,
                                 const QList<T>& l2)
{
    QList<T> r = l1;
    for (typename QList<T>::const_iterator i = l2.begin();
         i != l2.end(); ++i) {
        r.removeAll(*i);
    }
    return r;
}

namespace
{
    template <class T>
    class AutoArray
    {
    public:
        AutoArray(uint size): _ptr(new T[size]) {}
        operator T*() const { return _ptr; }
        ~AutoArray() { delete[] _ptr; }
    private:
        T* _ptr;
    };
}

QStringList Utilities::shuffle(const QStringList& list)
{
    static bool init = false;
    if ( !init ) {
        QTime midnight( 0, 0, 0 );
        srand( midnight.secsTo(QTime::currentTime()) );
        init = true;
    }

    // Take pointers from input list to an array for shuffling
    uint N = list.size();
    AutoArray<const QString*> deck(N);
    const QString** p = deck;
    for (QStringList::const_iterator i = list.begin();
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
    QStringList result;
    const QString** const onePastLast = deck + N;
    for (p = deck; p != onePastLast; ++p)
        result.push_back(**p);

    return result;
}

QStringList Utilities::diff( const QStringList& list1, const QStringList& list2 )
{
    QStringList result;
    for( QStringList::ConstIterator it = list1.constBegin(); it != list1.constEnd(); ++it ) {
        if ( !list2.contains( *it ) )
            result.append( *it );
    }
    return result;
}

QStringList Utilities::removeDuplicates( const QStringList& items )
{
    Set<QString> seen;
    QStringList res;
    for( QStringList::ConstIterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
        if ( !seen.contains( *itemIt ) ) {
            res.append( *itemIt );
            seen.insert( *itemIt );
        }
    }
    return res;
}


#define INSTANTIATE_MERGELISTSUNIQLY(T) \
template \
QList<T> Utilities::mergeListsUniqly(const QList<T>& l1, const QList<T>& l2)

#define INSTANTIATE_LISTSUBTRACT(T) \
template \
QList<T> Utilities::listSubtract(const QList<T>& l1, const QList<T>& l2)

INSTANTIATE_MERGELISTSUNIQLY(int);
INSTANTIATE_LISTSUBTRACT(int);
INSTANTIATE_LISTSUBTRACT(QString);

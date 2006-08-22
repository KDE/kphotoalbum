/*
  Copyright (C) 2006 Tuomas Suutari <thsuut@utu.fi>

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

#ifndef UTILITIES_GRAPH_H
#define UTILITIES_GRAPH_H

#include "Set.h"

template <class T>
QMap< T, Set<T> > pairsToMap(const QValueList<T[2]>& pairs);
template <class T>
QMap< T, Set<T> > closure(const QMap<T, Set<T> >& map);


template <class T>
QMap< T, Set<T> > pairsToMap(const QValueList<T[2]>& pairs)
{
    QMap< T, Set<T> > map;
    typename QValueList<T[2]>::const_iterator end = pairs.end();
    for (typename QValueList<T[2]>::const_iterator i = pairs.begin();
         i != end; ++i)
        map[(*i)[0]].insert((*i)[1]);
    return map;
}

template <class T>
QMap< T, Set<T> > closure(const QMap<T, Set<T> >& map)
{
    QMap< T, Set<T> > closure;
    QMap<T, bool> calculated;
    const QValueList<T> keys = map.keys();
    typename QValueList<T>::const_iterator keysEnd = keys.end();
    for (typename QValueList<T>::const_iterator i = keys.begin();
         i != keysEnd; ++i) {
        QValueList<T> queue;
        //closure[*i].insert(*i);
        queue.append(*i);
        Set<T> closure_i = closure[*i];
        while (!queue.empty()) {
            T x = queue.first();
            queue.pop_front();

            if (calculated[x]) { // optimization
                closure_i.insert(closure[x]);
                continue;
            }

            Set<T> adj = map[x];
            typename Set<T>::const_iterator adjEnd = adj.constEnd();
            for (typename Set<T>::const_iterator a = adj.constBegin();
                 a != adjEnd; ++a) {
                if (!closure_i.contains(*a)) {
                    queue.append(*a);
                    closure_i.insert(*a);
                }
            }
        }
        calculated[*i] = true;
    }
    return closure;
}

#endif /* UTILITIES_GRAPH_H */

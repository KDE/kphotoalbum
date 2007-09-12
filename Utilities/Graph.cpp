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

#include "Graph.h"

template <class T>
QMap< T, Utilities::Set<T> >
Utilities::pairsToMap(const QValueList< QPair<T, T> >& pairs)
{
    QMap< T, Utilities::Set<T> > map;
    typename QValueList< QPair<T, T> >::const_iterator end = pairs.end();
    for (typename QValueList< QPair<T, T> >::const_iterator i = pairs.begin();
         i != end; ++i)
        map[(*i).first].insert((*i).second);
    return map;
}

template <class T>
QMap< T, Utilities::Set<T> >
Utilities::closure(const QMap<T, Utilities::Set<T> >& map)
{
    QMap< T, Utilities::Set<T> > closure;
    QMap<T, bool> calculated;
    const QValueList<T> keys = map.keys();
    typename QValueList<T>::const_iterator keysEnd = keys.end();
    for (typename QValueList<T>::const_iterator i = keys.begin();
         i != keysEnd; ++i) {
        QValueList<T> queue;
        //closure[*i].insert(*i);
        queue.append(*i);
        Utilities::Set<T> closure_i = closure[*i];
        while (!queue.empty()) {
            T x = queue.first();
            queue.pop_front();

            if (calculated[x]) { // optimization
                closure_i += closure[x];
                continue;
            }

            Utilities::Set<T> adj = map[x];
            typename Utilities::Set<T>::const_iterator adjEnd = adj.end();
            for (typename Utilities::Set<T>::const_iterator a = adj.begin();
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


#define INSTANTIATE_PAIRSTOMAP(T) \
template \
QMap< T, Utilities::Set<T> > \
Utilities::pairsToMap(const QValueList< QPair<T, T> >& pairs)

#define INSTANTIATE_CLOSURE(T) \
template \
QMap< T, Utilities::Set<T> > \
Utilities::closure(const QMap<T, Utilities::Set<T> >& map)

INSTANTIATE_PAIRSTOMAP(QString);
INSTANTIATE_CLOSURE(QString);

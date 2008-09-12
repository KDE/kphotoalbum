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
#include <QLinkedList>

template <class T>
QMap< T, QSet<T> >
Utilities::pairsToMap(const QList< QPair<T, T> >& pairs)
{
    QMap< T, QSet<T> > map;
    typename QList< QPair<T, T> >::const_iterator end = pairs.end();
    for (typename QList< QPair<T, T> >::const_iterator i = pairs.begin();
         i != end; ++i)
        map[(*i).first].insert((*i).second);
    return map;
}

template <class T>
QMap< T, QSet<T> >
Utilities::closure(const QMap<T, QSet<T> >& map)
{
    QMap< T, QSet<T> > closure;
    QMap<T, bool> calculated;
    const QList<T> keys = map.keys();
    typename QList<T>::const_iterator keysEnd = keys.end();
    for (typename QList<T>::const_iterator i = keys.begin();
         i != keysEnd; ++i) {
        QLinkedList<T> queue;
        //closure[*i].insert(*i);
        queue.append(*i);
        QSet<T> closure_i = closure[*i];
        while (!queue.empty()) {
            T x = queue.first();
            queue.pop_front();

            if (calculated[x]) { // optimization
                closure_i += closure[x];
                continue;
            }

            QSet<T> adj = map[x];
            typename QSet<T>::const_iterator adjEnd = adj.end();
            for (typename QSet<T>::const_iterator a = adj.begin();
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
QMap< T, QSet<T> > \
Utilities::pairsToMap(const QList< QPair<T, T> >& pairs)

#define INSTANTIATE_CLOSURE(T) \
template \
QMap< T, QSet<T> > \
Utilities::closure(const QMap<T, QSet<T> >& map)

INSTANTIATE_PAIRSTOMAP(QString);
INSTANTIATE_CLOSURE(QString);

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

#ifndef UTILITIES_LIST_H
#define UTILITIES_LIST_H

namespace Utilities
{
    template <class T>
    QValueList<T> mergeListsUniqly(const QValueList<T>& l1,
                                   const QValueList<T>& l2);

    template <class T>
    QValueList<T> listSubtract(const QValueList<T>& l1,
                               const QValueList<T>& l2);



    // These are very simple and unoptimal implementations.
    // (Mainly because searching linked list is O(n).)
    // Could be optimized later, but no need for that yet.

    template <class T>
    QValueList<T> mergeListsUniqly(const QValueList<T>& l1,
                                   const QValueList<T>& l2)
    {
        QValueList<T> r;
        const QValueList<T>* l[2] = {&l1, &l2};
        for (int n = 0; n < 2; ++n)
            for (typename QValueList<T>::const_iterator i = l[n]->begin();
                 i != l[n]->end(); ++i)
                if (!r.contains(*i))
                    r.append(*i);
        return r;
    }

    template <class T>
    QValueList<T> listSubtract(const QValueList<T>& l1,
                               const QValueList<T>& l2)
    {
        QValueList<T> r = l1;
        for (typename QValueList<T>::const_iterator i = l2.begin();
             i != l2.end(); ++i) {
            r.remove(*i);
        }
        return r;
    }
}

#endif /* UTILITIES_LIST_H */

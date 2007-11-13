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

#include <qvaluelist.h>
#include <qstringlist.h>
#include <qvariant.h>

namespace Utilities
{
    /** Merge two lists to one list without duplicating items.
     *
     * Returned list will have items of l1 in original order followed
     * by those items of l2 that are not in l1.
     */
    template <class T>
    QValueList<T> mergeListsUniqly(const QValueList<T>& l1,
                                   const QValueList<T>& l2);

    /** Subtract a list from another list.
     *
     * Returned list will have those items of l1 that are not in l2,
     * in the original order of l1.
     */
    template <class T>
    QValueList<T> listSubtract(const QValueList<T>& l1,
                               const QValueList<T>& l2);

    /** Shuffle a list.
     *
     * Returned list will have same items as the given list, but in
     * random order.
     */
    template <class T>
    QValueList<T> shuffleList(const QValueList<T>& list);


    /** Copy some list to QValueList of QVariants.
     *
     * Class T should support iterating interface (e.g. const_iterator,
     * begin(), end()) and should be convertable to QVariant.
     *
     * @param l the list to copy from
     * @return list which contains elements of l in same order, but as QVariants
     */
    template <class T>
    QValueList<QVariant> toVariantList(const T& l);
}

#endif /* UTILITIES_LIST_H */

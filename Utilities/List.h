/*
  SPDX-FileCopyrightText: 2006-2010 Tuomas Suutari <thsuut@utu.fi>

  SPDX-License-Identifier: GPL-2.0-or-later

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

#include <QList>
#include <QVariant>

namespace Utilities
{
/** Merge two lists to one list without duplicating items.
 *
 * Returned list will have items of l1 in original order followed
 * by those items of l2 that are not in l1.
 */
template <class T>
QList<T> mergeListsUniqly(const QList<T> &l1, const QList<T> &l2);

/** Shuffle a list.
 *
 * Returned list will have same items as the given list, but in
 * random order.
 */
template <class T>
QList<T> shuffleList(const QList<T> &list);
}

#endif /* UTILITIES_LIST_H */
// vi:expandtab:tabstop=4 shiftwidth=4:

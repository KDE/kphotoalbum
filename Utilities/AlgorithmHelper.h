/* Copyright (C) 2013-2019 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ALGORITHMHELPER_H
#define ALGORITHMHELPER_H

#include <algorithm>

// The algotihms of std all takes two iterators, which makes it more verbose to use for the common case where we want the algortihm to work
// on the whole container. Below is a number of utility functions which makes it easier to use those algorithms.
// See http://en.cppreference.com/w/cpp/algorithm for a description of the C++ algorithmns.

namespace Utilities
{

template <class Container, class UnaryPredicate>
static bool any_of(const Container &container, UnaryPredicate p)
{
    return std::any_of(container.begin(), container.end(), p);
}

template <class Container, class UnaryPredicate>
static bool all_of(const Container &container, UnaryPredicate p)
{
    return std::all_of(container.begin(), container.end(), p);
}

/**
    Sum up the elements of the container. The value of each element is extracted using fn
 */
template <class Container, class ExtractFunction>
int sum(const Container &container, ExtractFunction fn)
{
    int res = 0;
    for (auto item : container) {
        res += fn(item);
    }
    return res;
}

}
#endif // ALGORITHMHELPER_H

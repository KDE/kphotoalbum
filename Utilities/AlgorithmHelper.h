#ifndef ALGORITHMHELPER_H
#define ALGORITHMHELPER_H

#include <algorithm>

// The algotihms of std all takes two iterators, which makes it more verbose to use for the common case where we want the algortihm to work
// on the whole container. Below is a number of utility functions which makes it easier to use those algorithms.
// See http://en.cppreference.com/w/cpp/algorithm for a description of the C++ algorithmns.

namespace Utilities {

template <class Container, class UnaryPredicate>
static bool any_of( const Container& container, UnaryPredicate p) {
    return std::any_of( container.begin(), container.end(), p);
}

template <class Container, class UnaryPredicate>
static bool all_of( const Container& container, UnaryPredicate p) {
    return std::all_of( container.begin(), container.end(), p);
}


/**
    Sum up the elements of the container. The value of each element is extracted using fn
 */
template <class Container, class ExtractFunction>
int sum(const Container& container, ExtractFunction fn ) {
    int res = 0;
    for ( auto item : container ) {
        res += fn(item);
    }
    return res;
}

}
#endif // ALGORITHMHELPER_H

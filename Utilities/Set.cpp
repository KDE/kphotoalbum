#include "Set.h"

bool Utilities::overlap( const StringSet& set1, const StringSet& set2 )
{
    StringSet tmp = set1;
    tmp.intersect(set2);
    return !tmp.isEmpty();
}


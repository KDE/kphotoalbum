#include "drawlist.h"
#include "draw.h"
DrawList::DrawList()
{
}

DrawList& DrawList::operator=( const DrawList& other )
{
    if ( this == &other )
        return *this;
    deleteItems();

    for( QValueList<Draw*>::ConstIterator it = other.begin(); it != other.end(); ++it ) {
        append( (*it)->clone() );
    }
    return *this;
}

DrawList::~DrawList()
{
    deleteItems();
}

void DrawList::deleteItems()
{
    // PENDING(blackie) Why does this code not work?
/*    for( QValueList<Draw*>::ConstIterator it = begin(); it != end();  ) {
        Draw* item = *it;
        ++it;
        delete item;
    }
*/
    clear();
}

#ifndef DRAWLIST_H
#define DRAWLIST_H
class Draw;
#include <qvaluelist.h>

class DrawList :public QValueList<Draw*>
{
public:
    DrawList();
    DrawList( const DrawList& other );
    ~DrawList();
    DrawList& operator=( const DrawList& other );
protected:
    void deleteItems();
};

#endif /* DRAWLIST_H */


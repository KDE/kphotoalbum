#ifndef DRAWLIST_H
#define DRAWLIST_H
class Draw;
class QWidget;
#include <qvaluelist.h>
#include <qdom.h>

class DrawList :public QValueList<Draw*>
{
public:
    DrawList();
    DrawList( const DrawList& other );
    ~DrawList();
    DrawList& operator=( const DrawList& other );
    void load( QDomElement elm );
    void save( QDomDocument doc, QDomElement top );
    void setWidget( QWidget* widget );

protected:
    void deleteItems();
};

#endif /* DRAWLIST_H */


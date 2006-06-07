#ifndef OUTLINE_H
#define OUTLINE_H
#include <qwidget.h>

namespace Video
{

class Outline :public QWidget
{
public:
    Outline( QWidget* parent );

protected:
    virtual void paintEvent( QPaintEvent* );
    virtual void resizeEvent( QResizeEvent* );

};

}


#endif /* OUTLINE_H */


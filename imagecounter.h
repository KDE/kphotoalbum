#ifndef IMAGECOUNTER_H
#define IMAGECOUNTER_H
#include <qlabel.h>

class ImageCounter :public QLabel {

public:
    ImageCounter( QWidget* parent, const char* name = 0 );
    void setPartial( int );
    void setTotal( int );
protected:
    void updateText();
private:
    int _partial, _total;
};


#endif /* IMAGECOUNTER_H */


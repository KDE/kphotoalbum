#ifndef IMAGECOUNTER_H
#define IMAGECOUNTER_H
#include <qlabel.h>

class ImageCounter :public QLabel {
    Q_OBJECT

public:
    ImageCounter( QWidget* parent, const char* name = 0 );

public slots:
    void setMatchCount( int start, int end, int matches );
    void setTotal( int );
    void showingOverview();
};


#endif /* IMAGECOUNTER_H */


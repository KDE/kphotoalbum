#ifndef VIEWER_H
#define VIEWER_H

#include <qdialog.h>
class ImageInfo;
class QLabel;

class Viewer :public QDialog
{
public:
    Viewer( QWidget* parent, const char* name = 0 );
    void load( ImageInfo* );

protected:
    virtual void resizeEvent( QResizeEvent* e );

private:
    QLabel* _label;
    ImageInfo* _info;
};

#endif /* VIEWER_H */


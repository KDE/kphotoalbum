#ifndef PIXMAPSNAPSHOT_H
#define PIXMAPSNAPSHOT_H

#include <qdialog.h>
#include <qpoint.h>

namespace Video
{
class Outline;

class PixmapSnapShot :public QDialog
{
    Q_OBJECT

public:
    PixmapSnapShot();

protected:
    void mousePressEvent( QMouseEvent* );
    void mouseMoveEvent( QMouseEvent* );
    void mouseReleaseEvent( QMouseEvent* );
    void showEvent( QShowEvent* );

protected slots:
    void doGrab();

private:
    Outline* _outline;
    QPoint _start, _end;
    bool _activated;
};

}

#endif /* PIXMAPSNAPSHOT_H */


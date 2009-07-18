#ifndef THUMBNAILDND_H
#define THUMBNAILDND_H
#include <QObject>
#include "ThumbnailComponent.h"
class QDragEnterEvent;
class QDropEvent;
class QDragLeaveEvent;
class QDragMoveEvent;

namespace ThumbnailView
{

class ThumbnailDND :public QObject, private ThumbnailComponent
{
    Q_OBJECT

public:
    ThumbnailDND( ThumbnailFactory* factory );
    void contentsDragMoveEvent( QDragMoveEvent* event );
    void contentsDragLeaveEvent( QDragLeaveEvent* );
    void contentsDropEvent( QDropEvent* );
    void contentsDragEnterEvent( QDragEnterEvent * event );

private slots:
    void realDropEvent();

private:
        void removeDropIndications();

};
}

#endif /* THUMBNAILDND_H */


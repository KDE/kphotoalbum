#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

class QPixmap;
class KFileItem;

#include <qvaluelist.h>
#include <qobject.h>

namespace ImageManager
{
class ImageRequest;

class VideoManager :public QObject
{
    Q_OBJECT

public:
    static VideoManager& instance();
    void request( ImageRequest* request );

protected:
    void load( ImageRequest* request );
    void requestLoadNext();

protected slots:
    void slotGotPreview(const KFileItem*, const QPixmap& pixmap );
    void previewFailed();

private:
    VideoManager();
    QValueList<ImageRequest*> _pending;
    ImageRequest* _currentRequest;
};

}

#endif /* VIDEOMANAGER_H */


#ifndef VIDEOLOADER_H
#define VIDEOLOADER_H

#include <kprocess.h>

namespace ImageManager
{
class ImageRequest;

class VideoLoader :QObject
{
    Q_OBJECT

public:
    VideoLoader( ImageRequest* request );
    void load( ImageRequest* );

protected:
    QString tmpDir() const;
    void startNextLoad();
    bool tryLoadThumbnail( ImageManager::ImageRequest* );
    void sendAnswer( QImage image, ImageManager::ImageRequest* request );
    QImage drawMovieClip( const QImage & );

protected slots:
    void processDone();


private:
    QValueList<ImageManager::ImageRequest*> _pendingRequest;
    ImageManager::ImageRequest* _current;

    KProcess _process;
};

}


#endif /* VIDEOLOADER_H */


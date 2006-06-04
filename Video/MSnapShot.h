#ifndef VIDEO_MSNAPSHOT_H
#define VIDEO_MSNAPSHOT_H

#include <kprocess.h>

namespace ImageManager { class ImageRequest; }

namespace Video
{
class MPlayer;

class MSnapShot :public QObject
{
    Q_OBJECT

public:
    void load( ImageManager::ImageRequest* );

protected:
    QString tmpDir() const;
    void startNextLoad();
    bool tryLoadThumbnail( ImageManager::ImageRequest* );
    void sendAnswer( QImage image, ImageManager::ImageRequest* request );

protected slots:
    void processDone();


private:
    friend class MPlayer;
    MSnapShot();
    QValueList<ImageManager::ImageRequest*> _pendingRequest;
    ImageManager::ImageRequest* _current;

    KProcess _process;
};

}

#endif /* VIDEO_MSNAPSHOT_H */


#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <qstringlist.h>
#include <qobject.h>
namespace ImageManager { class ImageRequest; }

namespace Video
{
class Player :public QObject
{
    Q_OBJECT

public:
    static Player* instance();
    virtual void play( const QStringList& files ) = 0;
    virtual void loadSnapshot( ImageManager::ImageRequest* ) = 0;

signals:
    void playing( bool );
};

}


#endif /* VIDEO_PLAYER_H */


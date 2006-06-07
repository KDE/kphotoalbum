#ifndef VIDEO_MPLAYER_H
#define VIDEO_MPLAYER_H

#include <qobject.h>
#include <qstringlist.h>
#include "Video/Player.h"
#include "MSnapShot.h"

class KProcess;

namespace Video
{
class Player;

class MPlayer :public Video::Player
{
    Q_OBJECT

public:
    virtual void play( const QStringList& files );
    void loadSnapshot( ImageManager::ImageRequest* );

protected slots:
    void startMovie();

private:
    friend class Player;
    MPlayer();
    KProcess* _process;
    QStringList _pendingMovies;
    MSnapShot _snapshot;
};

}

#endif /* VIDEO_MPLAYER_H */


#include "Player.h"
#include "MPlayer.h"

Video::Player* Video::Player::instance()
{
    static MPlayer* mplayer = 0;
    if ( mplayer == 0 )
        mplayer = new MPlayer;
    return mplayer;
}

#include "mysplashscreen.h"
#include <kstandarddirs.h>

MySplashScreen* MySplashScreen::_instance = 0;

MySplashScreen::MySplashScreen( )
    :KSplashScreen( locate("data", QString::fromLatin1("kimdaba/pics/splash-large.png") ) )
{
    _instance = this;
    QPixmap* pix = pixmap();
    resize( pix->size() );
}

MySplashScreen* MySplashScreen::instance()
{
    return _instance;
}

bool MySplashScreen::close( bool /*alsoDelete*/ )
{
    _instance = 0;
    return KSplashScreen::close( true );
}

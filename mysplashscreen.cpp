#include "mysplashscreen.h"
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <qpainter.h>

MySplashScreen* MySplashScreen::_instance = 0;

MySplashScreen::MySplashScreen()
    :KSplashScreen( locate("data", QString::fromLatin1("kimdaba/pics/splash-large.png") ) )
{
    _instance = this;
    QPixmap* pix = pixmap();
    resize( pix->size() );
    QPainter painter( pix );
    QFont fnt = font();
    fnt.setPixelSize( 24 );
    painter.setFont( fnt );
    QPen pen( white );
    painter.setPen( pen );
    painter.drawText( QRect( QPoint(260, 400), QPoint( 630, 470 )), AlignRight | AlignBottom,
                      i18n( "KimDaBa version %1" ).arg( KGlobal::instance()->aboutData()->version() ) );
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

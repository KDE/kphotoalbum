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
    fnt.setItalic( true );
    painter.setFont( fnt );
    QPen pen( white );
    painter.setPen( pen );
    QString txt;
    QString version = KGlobal::instance()->aboutData()->version();
    if ( version.startsWith( QString::fromLatin1("snap") ) )
        txt = i18n( "KimDaBa %1" ).arg( version );
    else
        txt = i18n( "KimDaBa version %1" ).arg( version );
    painter.drawText( QRect( QPoint(10, 400), QPoint( 630, 470 )), AlignRight | AlignBottom, txt );
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

#include "mysplashscreen.moc"

#include "infobox.h"
#include <qurl.h>
#include "viewer.h"
#include "browser.h"
InfoBox::InfoBox( Viewer* viewer, const char* name )
    :QTextBrowser( viewer, name ), _viewer( viewer )
{
    setFrameStyle( Box | Plain );
    setLineWidth(1);
    setMidLineWidth(0);
}

void InfoBox::setSource( const QString& which )
{
    int index = which.toInt();
    QPair<QString,QString> p = _linkMap[index];
    QString optionGroup = p.first;
    QString value = p.second;
    Browser::theBrowser()->load( optionGroup, value );
}

void InfoBox::setInfo( const QString& text, const QMap<int, QPair<QString,QString> >& linkMap )
{
    _linkMap = linkMap;
    setText( text );
    int width = 25;
    Q_ASSERT( isShown() ); // It seems like the contentsWidth() and contentsHeight() are not updated
                           // if the widget is not shown.
    do {
        width +=10;
        resize( width, width );
    } while ( contentsHeight() > contentsWidth() );
    resize( contentsWidth()+5, contentsHeight()+5 );
}

void InfoBox::contentsMouseMoveEvent( QMouseEvent* e)
{
    if ( e->state() & LeftButton ) {
        _viewer->infoBoxMove();
        // Do not tell QTextBrowser about the mouse movement, as this will just start a selection.
    }
    else
        QTextBrowser::contentsMouseMoveEvent( e );
}



/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "InfoBox.h"
#include <qurl.h>
#include "Viewer/Viewer.h"
#include "Browser/Browser.h"
#include <qfontmetrics.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "mainview.h"
#include "imageinfo.h"

Viewer::InfoBox::InfoBox( Viewer::Viewer* viewer, const char* name )
    :QTextBrowser( viewer, name ), _viewer( viewer )
{
    setFrameStyle( Box | Plain );
    setLineWidth(1);
    setMidLineWidth(0);

    _jumpToContext = new QToolButton( this );
    _jumpToContext->setIconSet( KGlobal::iconLoader()->loadIcon( QString::fromLatin1( "kphotoalbum" ), KIcon::Desktop, 16 ) );
    _jumpToContext->setFixedSize( 16, 16 );
    connect( _jumpToContext, SIGNAL( clicked() ), this, SLOT( jumpToContext() ) );
}

void Viewer::InfoBox::setSource( const QString& which )
{
    int index = which.toInt();
    QPair<QString,QString> p = _linkMap[index];
    QString category = p.first;
    QString value = p.second;
    Browser::Browser::instance()->load( category, value );
    showBrowser();
}

void Viewer::InfoBox::setInfo( const QString& text, const QMap<int, QPair<QString,QString> >& linkMap )
{
    _linkMap = linkMap;
    setText( text );
    setSize();
}

void Viewer::InfoBox::setSize()
{
    int width = 200;
    int height = 0, h2;

    do {
        width +=10;
        height = heightForWidth( width );
    } while ( height > width && width < _viewer->width()/3 );
    height = QMIN( height, _viewer->height()/3 );


    // make the box smaller in width till it fits
    int origWidth = width;
    do {
        width -= 10;
        h2 = heightForWidth( width );
        if ( width < 0 ) {
            // something went wrong - damn I hate this code
            break;
        }
    } while( height == h2 );
    if ( width < 0 )
        width = origWidth;
    else
        width+=10;

    width+=16; // space for the jump to context icon
    resize( width +4*frameWidth(), height +4*frameWidth());

    // Force the scrollbar off. This is to ensuer that we don't get in the situation where an image might have fited,
    // if it hadn't been because a scrollbar is shown
    setVScrollBarMode( AlwaysOff );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( Auto );
    setHScrollBarMode( Auto );

    int offset = 0;
    if ( verticalScrollBar()->isShown() )
        offset = verticalScrollBar()->width();
    _jumpToContext->move( width +3*frameWidth() - 16 - offset, frameWidth() );

}

void Viewer::InfoBox::contentsMouseMoveEvent( QMouseEvent* e)
{
    if ( e->state() & LeftButton ) {
        _viewer->infoBoxMove();
        // Do not tell QTextBrowser about the mouse movement, as this will just start a selection.
    }
    else
        QTextBrowser::contentsMouseMoveEvent( e );
}

void Viewer::InfoBox::jumpToContext()
{
    Browser::Browser::instance()->addImageView( _viewer->currentInfo()->fileName() );
    showBrowser();
}

void Viewer::InfoBox::showBrowser()
{
    QDesktopWidget* desktop = qApp->desktop();
    if ( desktop->screenNumber( Browser::Browser::instance() ) == desktop->screenNumber( _viewer ) ) {
        if (_viewer->showingFullScreen() )
            _viewer->setShowFullScreen( false );
        MainView::theMainView()->raise();
    }

}


#include "InfoBox.moc"

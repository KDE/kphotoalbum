/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "infobox.h"
#include <qurl.h>
#include "viewer.h"
#include "browser.h"
#include <qfontmetrics.h>
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
    int width = 200;
    int height = 0, h2;

    do {
        width +=10;
        height = heightForWidth( width );
    } while ( height > width && width < _viewer->width()/3 );
    height = QMIN( height, _viewer->height()/3 );


    // make the box smaller in width till it fits
    do {
        width -= 10;
        h2 = heightForWidth( width );
    } while( height == h2 );
    width+=10;

    resize( width +4*frameWidth(), height +4*frameWidth());

    // Force the scrollbar off. This is to ensuer that we don't get in the situation where an image might have fited,
    // if it hadn't been because a scrollbar is shown
    setVScrollBarMode( AlwaysOff );
    setHScrollBarMode( AlwaysOff );
    setVScrollBarMode( Auto );
    setHScrollBarMode( Auto );
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



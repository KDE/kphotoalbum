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

#include "mysplashscreen.h"
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <qpainter.h>
#include <qregexp.h>

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
    if ( QRegExp( QString::fromLatin1("[0-9.-]+") ).exactMatch( version ) )
        txt = i18n( "KimDaBa version %1" ).arg( version );
    else
        txt = i18n( "KimDaBa %1" ).arg( version );
    painter.drawText( QRect( QPoint(10, 400), QPoint( 630, 470 )), AlignRight | AlignBottom, txt );
}

MySplashScreen* MySplashScreen::instance()
{
    return _instance;
}

void MySplashScreen::done()
{
    _instance = 0;
    (void) close( true );
}

#include "mysplashscreen.moc"

/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "SplashScreen.h"
#include <KComponentData>
#include "Utilities/Util.h"
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <qpainter.h>
#include <qregexp.h>
#include <kdebug.h>

MainWindow::SplashScreen* MainWindow::SplashScreen::_instance = 0;

MainWindow::SplashScreen::SplashScreen()
    :KSplashScreen(Utilities::locateDataFile(QString::fromLatin1("pics/splash-large.png")))
{
    _instance = this;
}

MainWindow::SplashScreen* MainWindow::SplashScreen::instance()
{
    return _instance;
}

void MainWindow::SplashScreen::done()
{
    _instance = 0;
    (void) close();
    deleteLater();
}

void MainWindow::SplashScreen::message( const QString& message )
{
    _message = message;
    repaint();
}

void MainWindow::SplashScreen::drawContents( QPainter * painter )
{
    painter->save();
    QFont font = painter->font();
    font.setPointSize( 10 );
    painter->setFont( font );

    // Version String
    QString txt;
    QString version = KGlobal::mainComponent().aboutData()->version();

    if ( QRegExp( QString::fromLatin1("[0-9.-]+") ).exactMatch( version ) )
        txt = i18n( "Version %1" , version );
    else
        txt = i18n( "Version: %1" , version );
    painter->drawText( QRect( QPoint(230, 265), QSize( 150, 25 )), Qt::AlignRight | Qt::AlignTop, txt );

    // Message
    painter->drawText( QRect( QPoint(20, 265), QSize( 300, 25 )), Qt::AlignLeft | Qt::AlignTop, _message );
    painter->restore();
}

#include "SplashScreen.moc"

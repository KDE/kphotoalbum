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

#include "ImageCounter.h"
#include <klocale.h>
#include <QLabel>

MainWindow::ImageCounter::ImageCounter( QWidget* parent )
    :QLabel( parent  )
{
    setText( QString::fromLatin1( "---" ) );
    setMargin( 5 );
}

void MainWindow::ImageCounter::setMatchCount( uint matches )
{
    setText( i18np( "Showing 1 thumbnail", "Showing %1 thumbnails", matches ) );
}

void MainWindow::ImageCounter::setTotal( uint c )
{
    setText( i18n( "Total: %1", c) );
}

void MainWindow::ImageCounter::showBrowserMatches(uint matches )
{
    setText( i18np( "1 match", "%1 matches", matches ) );
}

#include "ImageCounter.moc"

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

#include "ImageCounter.h"
#include <klocale.h>

MainWindow::ImageCounter::ImageCounter( QWidget* parent, const char* name )
    :QLabel( parent, name )
{
    setText( QString::fromLatin1( "---" ) );
    setMargin( 5 );
}

void MainWindow::ImageCounter::setMatchCount( uint matches )
{
    setText( i18n( "Showing 1 image", "Showing %n images", matches ) );
}

void MainWindow::ImageCounter::setTotal( uint c )
{
    setText( i18n( "Total: %1" ).arg(c) );
}

void MainWindow::ImageCounter::showingOverview()
{
    setText( QString::fromLatin1( "---" ) );
}

#include "ImageCounter.moc"

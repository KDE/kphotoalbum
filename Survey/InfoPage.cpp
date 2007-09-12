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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "InfoPage.h"
#include "Utilities/Util.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>
#include <klocale.h>
#include <qpixmap.h>
#include <kstandarddirs.h>
Survey::InfoPage::InfoPage( const QString& appName, const QString& text, Location location, QWidget* parent, const char* name )
    :QWidget( parent, name )
{
    QVBoxLayout* vlay = new QVBoxLayout( this, 6 );
    QLabel* title;
    if ( location == Front )
        title = new QLabel( i18n( "<h1>%1 Survey</h1>" ).arg( appName ), this );
    else
        title = new QLabel( i18n( "<h1>%1 Survey Completed</h1>" ).arg( appName ), this );
    title->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    vlay->addWidget( title );

    QFrame* frame = new QFrame( this );
    frame->setFrameStyle( QFrame::HLine | QFrame::Plain );
    vlay->addWidget( frame );

    QHBoxLayout* hlay = new QHBoxLayout( vlay, 6 );

    QLabel* pict = new QLabel( this );
    if ( location == Front )
        pict->setPixmap(Utilities::locateDataFile(QString::fromLatin1("pics/questionmark-flipped.png")));
    else
        pict->setPixmap(Utilities::locateDataFile(QString::fromLatin1("pics/exclamationmark.png")));
    hlay->addWidget( pict );

    QLabel* desc = new QLabel( text, this );
    hlay->addWidget( desc, 1 );

    pict = new QLabel( this );
    if ( location == Front )
        pict->setPixmap(Utilities::locateDataFile(QString::fromLatin1("pics/questionmark.png")));
    else
        pict->setPixmap(Utilities::locateDataFile(QString::fromLatin1("pics/exclamationmark.png")));
    hlay->addWidget( pict );

    frame = new QFrame( this );
    frame->setFrameStyle( QFrame::HLine | QFrame::Plain );
    vlay->addWidget( frame );

}

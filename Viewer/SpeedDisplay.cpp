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

#include "Viewer/SpeedDisplay.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QHBoxLayout>

#include <klocale.h>

Viewer::SpeedDisplay::SpeedDisplay( QWidget* parent )
    :QDialog( parent )
{
    setWindowFlags( Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint );

    _label = new QLabel( this );
    _layout = new QHBoxLayout( this );
    _layout->setContentsMargins(0,0,0,0);
    _layout->addWidget( _label );
    _timer = new QTimer( this );
    connect( _timer, SIGNAL( timeout() ), this, SLOT( hide() ) );

    QPalette pal = _label->palette();
    _label->setAutoFillBackground( true );
    pal.setColor( QPalette::Background, Qt::yellow );
    _label->setPalette( pal );

    _label->setFrameStyle( Q3Frame::Box | Q3Frame::Plain );
}

void Viewer::SpeedDisplay::display( int i )
{
    _label->setText( i18n("<p><center><font size=\"+4\">%1&nbsp;s</font></center></p>", QString::number( i/1000.0, 'f', 1 ) ) );
    go();
}

void Viewer::SpeedDisplay::start( )
{
    _label->setText( i18n("<p><center><font size=\"+4\">Starting Slideshow</font></center></p>"));
    go();
}

void Viewer::SpeedDisplay::go()
{
    _layout->invalidate();
    resize( sizeHint() );
    QWidget* p = static_cast<QWidget*>( parent() );
    move( ( p->width() - width() )/2 + p->x(), ( p->height() - height() )/2 + p->y() );
    show();
    _timer->start( 1000 );
}

void Viewer::SpeedDisplay::end()
{
    _label->setText( i18n("<p><center><font size=\"+4\">Ending Slideshow</font></center></p>") );
    go();
}

#include "SpeedDisplay.moc"

/* Copyright (C) 2007-2010 Jan Kundrat <jkt@gentoo.org>

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

#include "TextDisplay.h"
#include <qlabel.h>
#include <qlayout.h>
#include <QLabel>
#include <QVBoxLayout>
#include "ImageDisplay.h"
#include "DB/ImageDB.h"

/**
 * Display a text instead of actual image/video data.
 */

Viewer::TextDisplay::TextDisplay( QWidget* parent )
    :AbstractDisplay( parent )
{
    QVBoxLayout *lay = new QVBoxLayout( this );
    _text = new QLabel( this );
    lay->addWidget( _text );
    _text->setAlignment( Qt::AlignCenter );

    QPalette pal = _text->palette();
    pal.setColor( QPalette::Background, Qt::white );
    _text->setPalette( pal );
}

bool Viewer::TextDisplay::setImage( DB::ImageInfoPtr info, bool forward )
{
    Q_UNUSED( info );
    Q_UNUSED( forward );
    return true;
}

void Viewer::TextDisplay::setText( const QString text )
{
    _text->setText( text );
}

#include "TextDisplay.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:

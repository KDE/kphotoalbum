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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "textquestion.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qdom.h>
Survey::TextQuestion::TextQuestion( const QString& id, const QString& title, const QString& question, SurveyDialog* parent )
    :Question( id, title, parent )
{
    QVBoxLayout* vlay = new QVBoxLayout( this, 6 );
    QLabel* label = new QLabel( question, this );
    vlay->addWidget( label );

    _edit = new QTextEdit( this );
    vlay->addWidget( _edit );
}

void Survey::TextQuestion::save( QDomElement& elm )
{
    QDomText txt = elm.ownerDocument().createTextNode( _edit->text() );
    elm.appendChild( txt );
}

void Survey::TextQuestion::load( QDomElement& elm )
{
    QDomNode node = elm.firstChild();
    if ( node.isText() )
        _edit->setText( node.toText().data() );
}

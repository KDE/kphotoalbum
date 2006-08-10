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

#include "Viewer/DrawList.h"
#include "Viewer/LineDraw.h"
#include "Viewer/RectDraw.h"
#include "Viewer/CircleDraw.h"
#include <kmessagebox.h>
#include <klocale.h>
Viewer::DrawList::DrawList() : QValueList<Viewer::Draw*>()
{
}

Viewer::DrawList& Viewer::DrawList::operator=( const Viewer::DrawList& other )
{
    if ( this == &other )
        return *this;
    deleteItems();

    for( QValueList<Viewer::Draw*>::ConstIterator it = other.begin(); it != other.end(); ++it ) {
        append( (*it)->clone() );
    }
    return *this;
}

Viewer::DrawList::~DrawList()
{
    deleteItems();
}

void Viewer::DrawList::deleteItems()
{
    for( QValueList<Draw*>::ConstIterator it = begin(); it != end();  ) {
        Draw* item = *it;
        ++it;
        delete item;
    }
    clear();
}

Viewer::DrawList::DrawList( const Viewer::DrawList& other )
    : QValueList<Draw*>()
{
    for( QValueList<Draw*>::ConstIterator it = other.begin(); it != other.end(); ++it ) {
        append( (*it)->clone() );
    }
}

void Viewer::DrawList::load( QDomElement elm )
{
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "drawings" ) );
    Q_ASSERT( count() == 0 );
    for ( QDomNode node = elm.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( !node.isElement() )
            continue;
        QDomElement child = node.toElement();

        QString tag = child.tagName();
        if ( tag == QString::fromLatin1( "Line" ) ) {
            append( new LineDraw( child ) );
        }
        else if ( tag == QString::fromLatin1( "Rectangle" ) ) {
            append( new RectDraw( child ) );
        }
        else if ( tag == QString::fromLatin1( "Circle" ) ) {
            append( new CircleDraw( child ) );
        }
        else {
            KMessageBox::error( 0, i18n("<qt><p>Unexpected element in configuration file: %1</p>"
                                        "<p>Expected one of: Line, Rectangle, Circle as sub element to Drawings</p></qt>" )
                                .arg(tag) );
        }
    }
}

void Viewer::DrawList::save( QDomDocument doc, QDomElement top )
{
    // Notice this method is used both from the XML DB and the .kim export.
    // If you ever change this code, pleas ensure that it is still
    // backwards compatible with older versions of the application (It
    // should be possible to export using a new version and read in an old version).
    if ( count() == 0 )
        return;

    QDomElement elm = doc.createElement( QString::fromLatin1( "drawings" ) );
    top.appendChild( elm );

    for( QValueList<Draw*>::iterator it = begin(); it != end(); ++it ) {
        elm.appendChild( (*it)->save( doc ) );
    }
}


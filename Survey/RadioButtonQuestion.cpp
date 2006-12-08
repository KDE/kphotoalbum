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

#include "radiobuttonquestion.h"
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qdom.h>
Survey::RadioButtonQuestion::RadioButtonQuestion( const QString& id, const QString& title, const QString& text,
                                                          const QString& question, const QStringList& questions, SurveyDialog* parent )
    :Question( id, title, parent )
{
    QHBoxLayout* hlay = new QHBoxLayout( this, 6 );
    QVBoxLayout* vlay;
    QLabel* label;

    if ( !text.isNull() ) {
        vlay = new QVBoxLayout( hlay, 6 );
        label = new QLabel( QString::fromLatin1("<p>%1</p>").arg(text), this );
        vlay->addWidget( label );
        vlay->addStretch( 1 );

        QFrame* frame = new QFrame( this );
        frame->setFrameStyle( QFrame::VLine | QFrame::Plain );
        hlay->addWidget( frame );
    }

    vlay = new QVBoxLayout( hlay, 6 );
    label = new QLabel( QString::fromLatin1("<h3>%1</h3>").arg(question), this );
    vlay->addWidget( label );

    _answers = new QButtonGroup( this );
    _answers->hide();

    for( QStringList::ConstIterator it = questions.begin(); it != questions.end(); ++it ) {
        QRadioButton* radio = new QRadioButton( *it, this );
        vlay->addWidget( radio );
        _answers->insert( radio );
    }
    vlay->addStretch( 1 );
}

void Survey::RadioButtonQuestion::save( QDomElement& elm )
{
    if ( _answers->selected() ) {
        elm.setAttribute( QString::fromLatin1( "text" ), _answers->selected()->text() );
        elm.setAttribute( QString::fromLatin1( "id" ), _answers->selectedId() );
    }
}

void Survey::RadioButtonQuestion::load( QDomElement& elm )
{
    int id = elm.attribute( QString::fromLatin1( "id" ), QString::fromLatin1( "-1" ) ).toInt();
    if ( id != -1 ) {
        QButton* but =_answers->find( id );
        if ( but )
            static_cast<QRadioButton*>(but)->setChecked( true );
    }
}

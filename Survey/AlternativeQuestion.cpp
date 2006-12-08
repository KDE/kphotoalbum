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
#include "AlternativeQuestion.h"
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qframe.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <klocale.h>
#include <qlineedit.h>
#include <qdom.h>
Survey::AlternativeQuestion::AlternativeQuestion( const QString& id, const QString& title, const QString& text,
                                                  const QString& question, const QStringList& questions, int otherCounts,
                                                  Type tp, SurveyDialog* parent )
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

    QButtonGroup* answers = new QButtonGroup( this );
    answers->hide();

    int index = 0;
    for( QStringList::ConstIterator it = questions.begin(); it != questions.end(); ++it, ++index ) {
        QButton* w;
        if (tp == RadioButton )
            w = new QRadioButton( *it, this, QString::number(index).latin1() );
        else
            w = new QCheckBox( *it, this, QString::number(index).latin1() );
        vlay->addWidget( w );
        answers->insert( w );
        _buttons.append( w );
    }

    for ( int i = 0; i < otherCounts; ++i ) {
        hlay = new QHBoxLayout( vlay, 6 );
        QLabel* label = new QLabel( i18n("Other: "), this );
        hlay->addWidget( label );
        QLineEdit* edit = new QLineEdit( this );
        hlay->addWidget( edit );
        _edits.append( edit );
    }

    vlay->addStretch( 1 );
}

void Survey::AlternativeQuestion::save( QDomElement& top )
{
    for( QValueList<QButton*>::Iterator buttonIt = _buttons.begin(); buttonIt != _buttons.end(); ++buttonIt ) {
        bool checked = false;
        QCheckBox* cb;
        QRadioButton* rb;
        if ( (cb = dynamic_cast<QCheckBox*>(*buttonIt) ) )
            checked = cb->isChecked();
        else if ( (rb = dynamic_cast<QRadioButton*>(*buttonIt) ) )
            checked = rb->isChecked();

        if ( checked ) {
            QDomElement elm = top.ownerDocument().createElement( QString::fromLatin1("ButtonAnswer") );
            elm.setAttribute( QString::fromLatin1( "text" ), (*buttonIt)->text() );
            elm.setAttribute( QString::fromLatin1( "index" ), QString::fromLocal8Bit( (*buttonIt)->name() ) );
            top.appendChild( elm );
        }
    }

    for( QValueList<QLineEdit*>::Iterator editIt = _edits.begin(); editIt != _edits.end(); ++editIt ) {
        if ( !(*editIt)->text().isEmpty() ) {
            QDomElement elm = top.ownerDocument().createElement( QString::fromLatin1("EditAnswer") );
            elm.setAttribute( QString::fromLatin1( "text" ), (*editIt)->text() );
            top.appendChild( elm );
        }
    }
}

void Survey::AlternativeQuestion::load( QDomElement& top )
{
    QValueList<QLineEdit*>::Iterator editIt = _edits.begin();
    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( node.isElement() ) {
            QDomElement elm = node.toElement();
            QString tag = elm.tagName();
            if ( tag == QString::fromLatin1( "ButtonAnswer" ) ) {
                QString index = elm.attribute( QString::fromLatin1( "index" ) );
                for( QValueList<QButton*>::Iterator buttonIt = _buttons.begin(); buttonIt != _buttons.end(); ++buttonIt ) {
                    if ( QString::fromLocal8Bit( (*buttonIt)->name() ) == index ) {
                        QCheckBox* cb;
                        QRadioButton* rb;
                        if ( (cb = dynamic_cast<QCheckBox*>(*buttonIt) ) )
                            cb->setChecked( true );
                        else if ( (rb = dynamic_cast<QRadioButton*>(*buttonIt) ) )
                            rb->setChecked( true );
                    }
                }
            }
            else if ( tag == QString::fromLatin1( "EditAnswer" ) ) {
                if ( editIt != _edits.end() ) {
                    (*editIt)->setText( elm.attribute( QString::fromLatin1( "text" ) ) );
                    editIt++;
                }
            }
        }
    }
}

Survey::RadioButtonQuestion::RadioButtonQuestion( const QString& id, const QString& title, const QString& text,
                                                  const QString& question, const QStringList& questions, SurveyDialog* parent )
    :AlternativeQuestion( id, title, text, question, questions, 0, AlternativeQuestion::RadioButton, parent )
{
}

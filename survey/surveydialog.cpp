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

#include "surveydialog.h"
#include <qlayout.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qvaluelist.h>
#include "question.h"
#include <qwidgetstack.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qdom.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <qprogressbar.h>

class SurveyPrivate
{
public:
    QWidgetStack* stack;
    QValueList<Survey::Question*> questions;
    int current;
    QPushButton* done;
    QPushButton* next;
    QPushButton* prev;
    QWidget* frontPage;
    QWidget* backPage;
    QMap<QString, Survey::Question*> questionMap;
    QString emailAddress;
    int surveyVersionMajor;
    int surveyVersionMinor;
    int invocationCount;
    int surveyVersionCompleted;
};

Survey::SurveyDialog::SurveyDialog( QWidget* parent, const char* name )
    : QDialog( parent, name, true )
{
    d = new SurveyPrivate();
    d->stack = 0;
    d->current = 0;
    d->frontPage = 0;
    d->invocationCount = 0;
    d->surveyVersionCompleted = 0;
}

Survey::SurveyDialog::~SurveyDialog()
{
    delete d;
}

void Survey::SurveyDialog::addQuestion( Question* question )
{
    d->questions.append( question );
    d->questionMap.insert( question->id(), question );
}

void Survey::SurveyDialog::exec()
{
    if ( !d->stack ) {
        QVBoxLayout* vlay = new QVBoxLayout( this, 6 );

        // Widget Stack
        d->stack = new QWidgetStack( this );
        vlay->addWidget( d->stack );

        setupFrontPage();

        int count = 0;
        for ( QValueList<Question*>::ConstIterator questionIt = d->questions.begin(); questionIt != d->questions.end();
              ++questionIt, ++count ) {
            d->stack->addWidget( createStackItem( *questionIt, count ), count+1 );
        }
        d->stack->raiseWidget(0);

        setupBackPage(count);

        QHBoxLayout* hlay = new QHBoxLayout( vlay, 6 );
        hlay->addStretch( 1 );

        d->prev = new QPushButton( i18n("<< Prev"), this );
        hlay->addWidget( d->prev );
        connect( d->prev, SIGNAL( clicked() ), this, SLOT( slotPrev() ) );

        d->next = new QPushButton( i18n("Next >>"), this );
        hlay->addWidget( d->next );
        connect( d->next, SIGNAL( clicked() ), this, SLOT( slotNext() ) );

        d->done = new QPushButton( this );
        connect(d->done, SIGNAL( clicked() ), this, SLOT( slotDone() ) );
        hlay->addWidget( d->done );

        d->next->setDefault( true );
        d->prev->setAutoDefault( false );
        d->done->setAutoDefault( false );

        readConfig();
    }

    goToPage(0);
    QDialog::exec();
}


QWidget* Survey::SurveyDialog::createStackItem( Question* question, int count )
{
    QWidget* w = new QWidget( d->stack );

    // Title
    QVBoxLayout* vlay = new QVBoxLayout( w, 6 );
    QHBoxLayout* hlay = new QHBoxLayout( vlay, 6 );
    QLabel* title = new QLabel( QString::fromLatin1("<qt><h1>%1</h1></qt>").arg(question->title()), w );
    hlay->addWidget( title, 1 );

    // Progress
    QProgressBar* progress = new QProgressBar( d->questions.count(), w );
    progress->setFixedWidth( 200 );
    hlay->addWidget( progress );
    progress->setProgress( count+1 );
    QLabel* label = new QLabel( QString::fromLatin1("%1/%2").arg( count+1 ).arg( d->questions.count() ), w );
    hlay->addWidget( label );

    // Line
    QFrame* frame = new QFrame( w );
    frame->setFrameStyle( QFrame::HLine | QFrame::Plain );
    vlay->addWidget( frame );

    question->reparent( w, 0, QPoint(0,0), true );
    vlay->addWidget( question );

    // Line
    frame = new QFrame( w );
    frame->setFrameStyle( QFrame::HLine | QFrame::Plain );
    vlay->addWidget( frame );
    return w;
}

void Survey::SurveyDialog::slotPrev()
{
    go(-1);
}

void Survey::SurveyDialog::slotNext()
{
    go(1);
}

void Survey::SurveyDialog::go( int direction )
{
    goToPage( d->current + direction );
}

void Survey::SurveyDialog::goToPage( int page )
{
    d->current = page;
    d->stack->raiseWidget(d->current);

    bool firstPage = (d->current == 0);

    d->prev->setEnabled( !firstPage );
    d->next->setEnabled( !lastPage() );

    if ( lastPage() )
        d->done->setText( i18n("Send Answers") );
    else
        d->done->setText( i18n("Pause Survey") );
}

void Survey::SurveyDialog::setFrontPage( QWidget* page )
{
    d->frontPage = page;
}

void Survey::SurveyDialog::setupFrontPage()
{
    if ( !d->frontPage )
        qFatal("Frontpage missing");

    d->frontPage->reparent( d->stack, 0, QPoint(0,0), true );
    d->stack->addWidget( d->frontPage, 0 );
}

void Survey::SurveyDialog::setBackPage( QWidget* page )
{
    d->backPage = page;
}

void Survey::SurveyDialog::setupBackPage( int count )
{
    if ( !d->backPage )
        qFatal("Backpage missing");

    d->backPage->reparent( d->stack, 0, QPoint(0,0), true );
    d->stack->addWidget( d->backPage, count+1 );
}

void Survey::SurveyDialog::slotDone()
{
    if ( lastPage() )
        d->surveyVersionCompleted = d->surveyVersionMajor;

    QCString xml = configAsXML();

    saveConfig( xml );

    if ( lastPage() ) {
        kapp->invokeMailer( d->emailAddress, QString::null, QString::null,
                            QString::fromLatin1( "%1 survey" ).arg(kapp->instanceName() ), xml );
    }

    close();
}

bool Survey::SurveyDialog::lastPage() const
{
    return (d->current ==  (int)d->questions.count()+1 );
}

void Survey::SurveyDialog::readConfig()
{
    QFile in( locateLocal( "appdata", "survey.xml" ) );
    if ( in.open( IO_ReadOnly ) ) {
        QDomDocument doc;
        doc.setContent(&in );
        QDomElement top = doc.documentElement();
        for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
            if ( !node.isElement() )
                continue;
            QDomElement elm = node.toElement();
            QString tag = elm.tagName();
            if ( d->questionMap.contains( tag ) )
                d->questionMap[tag]->load( elm );
        }

        d->invocationCount = top.attribute( QString::fromLatin1( "invocationCount") ).toInt() +1 ;
        d->surveyVersionCompleted = top.attribute( QString::fromLatin1( "surveyVersionCompleted" ) ).toInt();
    }

    saveConfig( configAsXML() ); // Save right away to increace invocation count
}

void Survey::SurveyDialog::saveConfig( const QCString& xml )
{
    QFile out( locateLocal( "appdata", "survey.xml" ) );
    if ( !out.open( IO_WriteOnly ) ) {
        Q_ASSERT( false );
    }
    else {
        out.writeBlock( xml.data(), xml.size()-1 );
        out.close();
    }
}

void Survey::SurveyDialog::setReceiver( const QString& emailAddress )
{
    d->emailAddress = emailAddress;
}

void Survey::SurveyDialog::setSurveyVersion( int major, int minor )
{
    d->surveyVersionMajor = major;
    d->surveyVersionMinor = minor;
}

void Survey::SurveyDialog::possibleExecSurvey( int minInvocations, int remindCount )
{
    Q_ASSERT( d->questions.count() != 0 );
    readConfig();
    if ( d->invocationCount >= minInvocations &&
         (d->invocationCount - minInvocations) % remindCount == 0 &&
         d->surveyVersionCompleted < d->surveyVersionMajor )
        exec();
}

QCString Survey::SurveyDialog::configAsXML()
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"),
                                                      QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement top = doc.createElement( QString::fromLatin1("survey") );
    top.setAttribute( QString::fromLatin1( "surveyVersionMinor" ), d->surveyVersionMinor );
    top.setAttribute( QString::fromLatin1( "surveyVersionMajor" ), d->surveyVersionMajor );
    top.setAttribute( QString::fromLatin1( "applicationVersion" ), KGlobal::instance()->aboutData()->version() );
    top.setAttribute( QString::fromLatin1( "invocationCount" ), d->invocationCount );
    top.setAttribute( QString::fromLatin1( "surveyVersionCompleted" ), d->surveyVersionCompleted );

    doc.appendChild( top );
    for ( QValueList<Question*>::ConstIterator questionIt = d->questions.begin(); questionIt != d->questions.end();
          ++questionIt ) {
        QDomElement elm = doc.createElement( (*questionIt)->id() );
        (*questionIt)->save( elm );
        top.appendChild( elm );
    }

    return doc.toCString();
}

#include "surveydialog.moc"

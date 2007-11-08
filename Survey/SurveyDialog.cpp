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

#include "SurveyDialog.h"
#include <qlayout.h>
#include <qlabel.h>
#include <q3valuelist.h>
#include "Question.h"
#include <q3widgetstack.h>

#include <QHBoxLayout>
#include <Q3CString>
#include <Q3Frame>
#include <QVBoxLayout>
#include <klocale.h>
#include <qpushbutton.h>
#include <qdom.h>
#include <kstandarddirs.h>
#include <qfile.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <q3progressbar.h>
#include <ktoolinvocation.h>
#include <KCmdLineArgs>

class SurveyPrivate
{
public:
    Q3WidgetStack* stack;
    Q3ValueList<Survey::Question*> questions;
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

Survey::SurveyDialog::SurveyDialog( QWidget* parent )
    : QDialog( parent )
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
        QVBoxLayout* vlay = new QVBoxLayout( this );

        // Widget Stack
        d->stack = new Q3WidgetStack( this );
        vlay->addWidget( d->stack );

        setupFrontPage();

        int count = 0;
        for ( Q3ValueList<Question*>::ConstIterator questionIt = d->questions.begin(); questionIt != d->questions.end();
              ++questionIt, ++count ) {
            d->stack->addWidget( createStackItem( *questionIt, count ), count+1 );
        }
        d->stack->raiseWidget(0);

        setupBackPage(count);

        QHBoxLayout* hlay = new QHBoxLayout;
        vlay->addLayout( hlay );

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
    QVBoxLayout* vlay = new QVBoxLayout( w );
    QHBoxLayout* hlay = new QHBoxLayout;
    vlay->addLayout( hlay );

    QLabel* title = new QLabel( QString::fromLatin1("<h1>%1</h1>").arg(question->title()), w );
    hlay->addWidget( title, 1 );

    // Progress
    Q3ProgressBar* progress = new Q3ProgressBar( d->questions.count(), w );
    progress->setFixedWidth( 200 );
    hlay->addWidget( progress );
    progress->setProgress( count+1 );
    QLabel* label = new QLabel( QString::fromLatin1("%1/%2").arg( count+1 ).arg( d->questions.count() ), w );
    hlay->addWidget( label );

    // Line
    Q3Frame* frame = new Q3Frame( w );
    frame->setFrameStyle( Q3Frame::HLine | Q3Frame::Plain );
    vlay->addWidget( frame );

    question->setParent( w );
    vlay->addWidget( question );

    // Line
    frame = new Q3Frame( w );
    frame->setFrameStyle( Q3Frame::HLine | Q3Frame::Plain );
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

    d->frontPage->setParent( d->stack );
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

    d->backPage->setParent( d->stack );
    d->stack->addWidget( d->backPage, count+1 );
}

void Survey::SurveyDialog::slotDone()
{
    if ( lastPage() )
        d->surveyVersionCompleted = d->surveyVersionMajor;

    Q3CString xml = configAsXML();

    saveConfig( xml );

    if ( lastPage() ) {
        KToolInvocation::invokeMailer( d->emailAddress, QString::null, QString::null,
                            QString::fromLatin1( "KPhotoAlbum survey" ), xml );
    }

    close();
}

bool Survey::SurveyDialog::lastPage() const
{
    return (d->current ==  (int)d->questions.count()+1 );
}

void Survey::SurveyDialog::readConfig()
{
    QFile in( KStandardDirs::locateLocal( "appdata", QString::fromLatin1("survey.xml") ) );
    if ( in.open( QIODevice::ReadOnly ) ) {
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

void Survey::SurveyDialog::saveConfig( const Q3CString& xml )
{
    QFile out( KStandardDirs::locateLocal( "appdata", QString::fromLatin1("survey.xml") ) );
    if ( !out.open( QIODevice::WriteOnly ) ) {
        Q_ASSERT( false );
    }
    else {
        out.write( xml.data(), xml.size()-1 );
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

QByteArray Survey::SurveyDialog::configAsXML()
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"),
                                                      QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement top = doc.createElement( QString::fromLatin1("survey") );
    top.setAttribute( QString::fromLatin1( "surveyVersionMinor" ), d->surveyVersionMinor );
    top.setAttribute( QString::fromLatin1( "surveyVersionMajor" ), d->surveyVersionMajor );
    top.setAttribute( QString::fromLatin1( "applicationVersion" ), KCmdLineArgs::aboutData()->version() );
    top.setAttribute( QString::fromLatin1( "invocationCount" ), d->invocationCount );
    top.setAttribute( QString::fromLatin1( "surveyVersionCompleted" ), d->surveyVersionCompleted );

    doc.appendChild( top );
    for ( Q3ValueList<Question*>::ConstIterator questionIt = d->questions.begin(); questionIt != d->questions.end();
          ++questionIt ) {
        QDomElement elm = doc.createElement( (*questionIt)->id() );
        (*questionIt)->save( elm );
        top.appendChild( elm );
    }

    return doc.toByteArray();
}

#include "SurveyDialog.moc"

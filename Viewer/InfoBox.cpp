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

#include "InfoBox.h"
#include "Browser/BrowserWidget.h"
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qcursor.h>
#include <QMouseEvent>
#include <kglobal.h>
#include <kiconloader.h>
#include "MainWindow/Window.h"
#include "DB/ImageInfo.h"
#include <QDesktopWidget>
#include <kdebug.h>

Viewer::InfoBox::InfoBox( Viewer::ViewerWidget* viewer )
    :QTextBrowser( viewer ), _viewer( viewer ), _hoveringOverLink( false )
{
    setFrameStyle( Box | Plain );
    setLineWidth(1);
    setMidLineWidth(0);

    _jumpToContext = new QToolButton( this );
    _jumpToContext->setIcon( KIcon( QString::fromLatin1( "kphotoalbum" ) ) );
    _jumpToContext->setFixedSize( 16, 16 );
    connect( _jumpToContext, SIGNAL( clicked() ), this, SLOT( jumpToContext() ) );
    connect( this, SIGNAL( highlighted(const QString&) ),
             SLOT( linkHovered(const QString&) ));
}

void Viewer::InfoBox::setSource( const QString& which )
{
    int index = which.toInt();
    QPair<QString,QString> p = _linkMap[index];
    QString category = p.first;
    QString value = p.second;
    Browser::BrowserWidget::instance()->load( category, value );
    showBrowser();
}

void Viewer::InfoBox::setInfo( const QString& text, const QMap<int, QPair<QString,QString> >& linkMap )
{
    _linkMap = linkMap;
    setText( text );
    setSize();
}

void Viewer::InfoBox::setSize()
{
    int width = viewport()->width();
    int height = 0, h2;

    do {
        width +=10;
        height = heightForWidth( width );
    } while ( height > width && width < _viewer->width()/3 );
    height = qMin( height, _viewer->height()/3 );


    // make the box smaller in width till it fits
    int origWidth = width;
    do {
        width -= 10;
        h2 = heightForWidth( width );
        if ( width < 0 ) {
            // something went wrong - damn I hate this code
            break;
        }
    } while( height == h2 );
    if ( width < 0 )
        width = origWidth;
    else
        width+=10;

    width+=16; // space for the jump to context icon
    resize( width +4*frameWidth(), height +4*frameWidth());

    // Force the scrollbar off. This is to ensuer that we don't get in the situation where an image might have fited,
    // if it hadn't been because a scrollbar is shown
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );

    int offset = 0;
    if ( !verticalScrollBar()->isHidden() )
        offset = verticalScrollBar()->width();
    _jumpToContext->move( width +3*frameWidth() - 16 - offset, frameWidth() );

}

void Viewer::InfoBox::mousePressEvent( QMouseEvent* e )
{
    // if we are just over a link, don't change the cursor to 'movement':
    // that would be irritating
    if (!_hoveringOverLink)
        viewport()->setCursor( Qt::SizeAllCursor );
    QTextBrowser::mousePressEvent(e);
}

void Viewer::InfoBox::mouseReleaseEvent( QMouseEvent* e )
{
    if (!_hoveringOverLink)
        viewport()->unsetCursor();
    QTextBrowser::mouseReleaseEvent(e);
}

void Viewer::InfoBox::mouseMoveEvent( QMouseEvent* e)
{
    if ( e->buttons() & Qt::LeftButton ) {
        _viewer->infoBoxMove();
        // Do not tell QTextBrowser about the mouse movement, as this will just start a selection.
    }
    else
        QTextBrowser::mouseMoveEvent( e );
}

void Viewer::InfoBox::linkHovered( const QString& linkName )
{
    _hoveringOverLink = !linkName.isNull();
}

void Viewer::InfoBox::jumpToContext()
{
    Browser::BrowserWidget::instance()->addImageView( _viewer->currentInfo()->fileName() );
    showBrowser();
}

void Viewer::InfoBox::showBrowser()
{
    QDesktopWidget* desktop = qApp->desktop();
    if ( desktop->screenNumber( Browser::BrowserWidget::instance() ) == desktop->screenNumber( _viewer ) ) {
        if (_viewer->showingFullScreen() )
            _viewer->setShowFullScreen( false );
        MainWindow::Window::theMainWindow()->raise();
    }

}


#include "InfoBox.moc"

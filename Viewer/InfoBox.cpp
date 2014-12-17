/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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

#include "InfoBox.h"
#include <KActionCollection>
#include "VisibleOptionsMenu.h"
#include <qglobal.h>
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
#include "DB/ImageDB.h"
#include <qscrollbar.h>
#include <QBitmap>

using namespace Settings;

Viewer::InfoBox::InfoBox( Viewer::ViewerWidget* viewer )
    :KTextBrowser( viewer ), m_viewer( viewer ), m_hoveringOverLink( false ), m_infoBoxResizer( this ), m_menu(nullptr)
{
    setFrameStyle( Box | Plain );
    setLineWidth(1);
    setMidLineWidth(0);
    setAutoFillBackground(false);


    QPalette p = palette();
    p.setColor(QPalette::Base, QColor(0,0,0,170)); // r,g,b,A
    p.setColor(QPalette::Text, Qt::white );
    p.setColor(QPalette::Link, QColor(Qt::blue).light() );
    setPalette(p);

    m_jumpToContext = new QToolButton( this );
    m_jumpToContext->setIcon( KIcon( QString::fromLatin1( "kphotoalbum" ) ) );
    m_jumpToContext->setFixedSize( 16, 16 );
    connect( m_jumpToContext, SIGNAL(clicked()), this, SLOT(jumpToContext()) );
    connect( this, SIGNAL(highlighted(QString)),
             SLOT(linkHovered(QString)));
    m_jumpToContext->setCursor( Qt::ArrowCursor );

    KRatingWidget* rating = new KRatingWidget( nullptr );

    // Unfortunately, the KRatingWidget now thinks that it has some absurdly big
    // dimensions. This call will persuade it to stay reasonably small.
    QPixmap::grabWidget( rating );

    for ( int i = 0; i <= 10; ++i ) {
        rating->setRating( i );
        // Workaround for http://trolltech.no/developer/task-tracker/index_html?method=entry&id=142869
        // There's no real transparency in grabWidget() :(
        QPixmap pixmap = QPixmap::grabWidget( rating );
        pixmap.setMask( pixmap.createHeuristicMask() );
        m_ratingPixmap.append( pixmap ) ;
    }
    delete rating;
}

QVariant Viewer::InfoBox::loadResource( int type, const QUrl& name )
{
    if ( name.scheme() == QString::fromLatin1( "KRatingWidget" ) ) {
        short int rating = name.host().toShort();
        return m_ratingPixmap[ rating ];
    }
    return KTextBrowser::loadResource( type, name );
}

void Viewer::InfoBox::setSource( const QUrl& which )
{
    int index = which.path().toInt();
    QPair<QString,QString> p = m_linkMap[index];
    QString category = p.first;
    QString value = p.second;
    Browser::BrowserWidget::instance()->load( category, value );
    showBrowser();
}

void Viewer::InfoBox::setInfo( const QString& text, const QMap<int, QPair<QString,QString> >& linkMap )
{
    m_linkMap = linkMap;
    setText( text );

    hackLinkColorForQt44();

    setSize();
}

void Viewer::InfoBox::setSize()
{
    const int maxWidth = Settings::SettingsData::instance()->infoBoxWidth();
    const int maxHeight = Settings::SettingsData::instance()->infoBoxHeight();

    document()->setPageSize( QSize(maxWidth, maxHeight) );
#if 1
    bool showVerticalBar = document()->size().height() > maxHeight;

    setVerticalScrollBarPolicy( showVerticalBar ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    const int realWidth =
        static_cast<int>(document()->idealWidth()) +
        (showVerticalBar ? verticalScrollBar()->width() + frameWidth() : 0) +
        m_jumpToContext->width() + 10;

    resize( realWidth, qMin( (int)document()->size().height(), maxHeight ) );
#else
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    resize( maxWidth, maxHeight );
#endif
}

void Viewer::InfoBox::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
        possiblyStartResize( e->pos() );
    KTextBrowser::mousePressEvent(e);
}

void Viewer::InfoBox::mouseReleaseEvent( QMouseEvent* e )
{
    if ( m_infoBoxResizer.isActive() ) {
        Settings::SettingsData::instance()->setInfoBoxWidth( width() );
        Settings::SettingsData::instance()->setInfoBoxHeight( height() );
    }

    m_infoBoxResizer.deactivate();
    KTextBrowser::mouseReleaseEvent(e);
}

void Viewer::InfoBox::mouseMoveEvent( QMouseEvent* e)
{
    if ( e->buttons() & Qt::LeftButton ) {
        if ( m_infoBoxResizer.isActive() )
            m_infoBoxResizer.setPos( e->pos() );
        else
            m_viewer->infoBoxMove();
        // Do not tell KTextBrowser about the mouse movement, as this will just start a selection.
    }
    else {
        updateCursor(e->pos() );
        KTextBrowser::mouseMoveEvent( e );
    }
}

void Viewer::InfoBox::linkHovered( const QString& linkName )
{
    if (linkName == QString()) {
        emit noTagHovered();
    } else {
        emit tagHovered(m_linkMap[linkName.toInt()]);
    }

    m_hoveringOverLink = !linkName.isNull();
}

void Viewer::InfoBox::jumpToContext()
{
    Browser::BrowserWidget::instance()->addImageView( m_viewer->currentInfo()->fileName() );
    showBrowser();
}

void Viewer::InfoBox::showBrowser()
{
    QDesktopWidget* desktop = qApp->desktop();
    if ( desktop->screenNumber( Browser::BrowserWidget::instance() ) == desktop->screenNumber( m_viewer ) ) {
        if (m_viewer->showingFullScreen() )
            m_viewer->setShowFullScreen( false );
        MainWindow::Window::theMainWindow()->raise();
    }

}



/**
 * Update the cursor based on the cursors position in the info box
 */
void Viewer::InfoBox::updateCursor( const QPoint& pos )
{
    const int border = 25;

    bool left = (pos.x() < border);
    bool right = pos.x() > width()-border;
    bool top = pos.y() < border;
    bool bottom = pos.y() > height()-border;

    Settings::Position windowPos = Settings::SettingsData::instance()->infoBoxPosition();

    Qt::CursorShape shape = Qt::SizeAllCursor;
    if ( m_hoveringOverLink ) shape = Qt::PointingHandCursor;
    else if ( atBlackoutPos( left, right, top, bottom, windowPos ) )
        shape = Qt::SizeAllCursor;
    else if ( ( left && top ) || ( right && bottom ) ) shape = Qt::SizeFDiagCursor;
    else if ( ( left && bottom ) || ( right && top ) ) shape = Qt::SizeBDiagCursor;
    else if ( top || bottom ) shape = Qt::SizeVerCursor;
    else if ( left || right ) shape = Qt::SizeHorCursor;

    setCursor( shape );
    viewport()->setCursor( shape );
}

/**
 * Return true if we are at an edge of the image info box that is towards the edge of the viewer.
 * We can forexample not make the box taller at the bottom if the box is sitting at the bottom of the viewer.
 */
bool Viewer::InfoBox::atBlackoutPos( bool left, bool right, bool top, bool bottom, Settings::Position pos ) const
{
    return ( left && (pos == Left || pos == TopLeft || pos == BottomLeft ) ) ||
        ( right && (pos == Right || pos == TopRight || pos == BottomRight ) ) ||
        ( top && ( pos == Top || pos == TopLeft || pos == TopRight ) ) ||
        ( bottom && ( pos == Bottom || pos == BottomLeft || pos == BottomRight ) );
}

void Viewer::InfoBox::possiblyStartResize( const QPoint& pos )
{
    const int border = 25;

    bool left = (pos.x() < border);
    bool right = pos.x() > width()-border;
    bool top = pos.y() < border;
    bool bottom = pos.y() > height()-border;

    if ( left || right || top || bottom )
        m_infoBoxResizer.setup(left,right,top,bottom);
}

void Viewer::InfoBox::resizeEvent( QResizeEvent* )
{
    QPoint pos = viewport()->rect().adjusted(0,2,-m_jumpToContext->width()-2,0).topRight();
    m_jumpToContext->move( pos );
}

void Viewer::InfoBox::hackLinkColorForQt44()
{

    QTextCursor cursor(document());
    Q_FOREVER {
        QTextCharFormat f = cursor.charFormat();
        if (f.isAnchor()) {
            f.setForeground(QColor(Qt::blue).light());
            QTextCursor c2 = cursor;
            c2.movePosition( QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor );
            c2.setCharFormat(f);
        }
        if ( cursor.atEnd() ) {
            break;
        }
        cursor.movePosition( QTextCursor::NextCharacter );
    }
}

void Viewer::InfoBox::contextMenuEvent( QContextMenuEvent* event )
{
    if ( !m_menu ) {
        m_menu = new VisibleOptionsMenu( m_viewer, new KActionCollection((QObject*)nullptr) );
        connect( m_menu, SIGNAL(visibleOptionsChanged()), m_viewer, SLOT(updateInfoBox()) );
    }
    m_menu->exec(event->globalPos());
}

#include "InfoBox.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:

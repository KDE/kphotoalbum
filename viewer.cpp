/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "viewer.h"
#include <qlayout.h>
#include <qlabel.h>
#include "imageinfo.h"
#include "imagemanager.h"
#include <qsizepolicy.h>
#include <qsimplerichtext.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qrect.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qaction.h>
#include "displayarea.h"
#include <qtoolbar.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kaction.h>
#include <klocale.h>
#include "util.h"
#include <qsignalmapper.h>
#include "showoptionaction.h"
#include <qtimer.h>
#include "drawhandler.h"
#include <kwin.h>
#include <kglobalsettings.h>

Viewer* Viewer::_latest = 0;

Viewer* Viewer::latest()
{
    return _latest;
}

Viewer::Viewer( QWidget* parent, const char* name )
    :QDialog( parent,  name ), _showingFullScreen( false )
{
    _latest = this;

    QVBoxLayout* layout = new QVBoxLayout( this );

    _display = new DisplayArea( this ); // Must be created before the toolbar.
    createToolBar();
    _toolbar->hide();


    layout->addWidget( _toolbar );
    layout->addWidget( _display );

    // This must not be added to the layout, as it is standing on top of
    // the DisplayArea
    _infoBox = new InfoBox( this );
    _infoBox->setShown( Options::instance()->showInfoBox() );

    setupContextMenu();
}


void Viewer::setupContextMenu()
{
    _popup = new QPopupMenu( this );
    QAction* action;

    _firstAction = new QAction( i18n("First"), QIconSet(), i18n("First"), Key_Home, this );
    connect( _firstAction,  SIGNAL( activated() ), this, SLOT( showFirst() ) );
    _firstAction->addTo( _popup );

    _lastAction = new QAction( i18n("Last"), QIconSet(), i18n("Last"), Key_End, this );
    connect( _lastAction,  SIGNAL( activated() ), this, SLOT( showLast() ) );
    _lastAction->addTo( _popup );

    _nextAction = new QAction( i18n("Show Next"), QIconSet(), i18n("Show Next"), Key_PageDown, this );
    connect( _nextAction,  SIGNAL( activated() ), this, SLOT( showNext() ) );
    _nextAction->addTo( _popup );

    _prevAction = new QAction( i18n("Show Previous"),  QIconSet(), i18n("Show Previous"), Key_PageUp, this );
    connect( _prevAction,  SIGNAL( activated() ), this, SLOT( showPrev() ) );
    _prevAction->addTo( _popup );

    _popup->insertSeparator();

    action = new QAction( i18n("Zoom In"),  QIconSet(), i18n("Zoom In"), Key_Plus, this );
    connect( action,  SIGNAL( activated() ), _display, SLOT( zoomIn() ) );
    action->addTo( _popup );

    action = new QAction( i18n("Zoom Out"),  QIconSet(), i18n("Zoom Out"), Key_Minus, this );
    connect( action,  SIGNAL( activated() ), _display, SLOT( zoomOut() ) );
    action->addTo( _popup );

    action = new QAction( i18n("Toggle Full Screen"),  QIconSet(), i18n("Toggle Full Screen"), Key_Space, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( toggleFullScreen() ) );
    action->addTo( _popup );

    _popup->insertSeparator();

    action = new QAction( i18n("Rotate 90 Degrees"),  QIconSet(), i18n("Rotate 90 Degrees"), Key_9, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( rotate90() ) );
    action->addTo( _popup );

    action = new QAction( i18n("Rotate 180 Degrees"),  QIconSet(), i18n("Rotate 180 Degrees"), Key_8, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( rotate180() ) );
    action->addTo( _popup );

    action = new QAction( i18n("Rotate 270 Degrees"),  QIconSet(), i18n("Rotate 270 degrees"), Key_7, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( rotate270() ) );
    action->addTo( _popup );

    _popup->insertSeparator();

    action = new QAction( i18n("Show Info Box"), QIconSet(), i18n("Show Info Box"), Key_I, this, "showInfoBox", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowInfoBox( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showInfoBox() );

    action = new QAction( i18n("Show Drawing"), QIconSet(), i18n("Show Drawing"), CTRL+Key_I, this, "showDrawing", true );
    connect( action, SIGNAL( toggled( bool ) ), _display, SLOT( toggleShowDrawings( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showDrawings() );

    action = new QAction( i18n("Show Description"), QIconSet(), i18n("Show Description"), Key_D, this, "showDescription", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDescription( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showDescription() );

    action = new QAction( i18n("Show Date"), QIconSet(), i18n("Show Date"), Key_A, this, "showDate", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDate( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showDate() );

    QStringList grps = Options::instance()->optionGroups();

    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        action = new ShowOptionAction( *it, this );
        action->addTo( _popup );
        connect( action, SIGNAL( toggled( const QString&, bool ) ),
                 this, SLOT( toggleShowOption( const QString&, bool ) ) );
    }

    _popup->insertSeparator();

    action = new QAction( i18n("Draw on Image"),  QIconSet(),  i18n("Draw on Image"),  0, this );
    connect( action,  SIGNAL( activated() ),  this, SLOT( startDraw() ) );
    action->addTo( _popup );

    action = new QAction( i18n("Close"),  QIconSet(), i18n("Close"), Key_Q, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( close() ) );
    action->addTo( _popup );
}

void Viewer::load( const ImageInfoList& list, int index )
{
    _list = list;
    _current = index;
    load();
}

void Viewer::load()
{
    _display->drawHandler()->setDrawList( currentInfo()->drawList() );
    _display->setImage( currentInfo() );
    updateInfoBox();

    _nextAction->setEnabled( _current +1 < (int) _list.count() );
    _prevAction->setEnabled( _current > 0 );
    _firstAction->setEnabled( _current > 0 );
    _lastAction->setEnabled( _current +1 < (int) _list.count() );
}

void Viewer::contextMenuEvent( QContextMenuEvent * e )
{
    _popup->exec( e->globalPos() );
    e->accept();
}

void Viewer::showNext()
{
    save();
    if ( _current +1 < (int) _list.count() )  {
        _current++;
        load();
    }
}

void Viewer::showPrev()
{
    save();
    if ( _current > 0  )  {
        _current--;
        load();
    }
}

void Viewer::rotate90()
{
    currentInfo()->rotate( 90 );
    load();
}

void Viewer::rotate180()
{
    currentInfo()->rotate( 180 );
    load();
}

void Viewer::rotate270()
{
    currentInfo()->rotate( 270 );
    load();
}

void Viewer::toggleShowInfoBox( bool b )
{
    Options::instance()->setShowInfoBox( b );
    _infoBox->setShown(b);
}

void Viewer::toggleShowDescription( bool b )
{
    Options::instance()->setShowDescription( b );
    updateInfoBox();
}

void Viewer::toggleShowDate( bool b )
{
    Options::instance()->setShowDate( b );
    updateInfoBox();
}

void Viewer::toggleShowOption( const QString& optionGroup, bool b )
{
    Options::instance()->setShowOption( optionGroup, b );
    updateInfoBox();
}

void Viewer::showFirst()
{
    save();
    _current = 0;
    load();
}

void Viewer::showLast()
{
    save();
     _current = _list.count() -1;
     load();
}

void Viewer::closeEvent( QCloseEvent* )
{
    close();
}

void Viewer::save()
{
    currentInfo()->setDrawList( _display->drawHandler()->drawList() );
}

void Viewer::startDraw()
{
    _display->startDrawing();
    _display->drawHandler()->slotSelect();
    _toolbar->show();
}

void Viewer::stopDraw()
{
    _display->stopDrawing();
    _toolbar->hide();
}

void Viewer::close()
{
    save();
    hide();
}

ImageInfo* Viewer::currentInfo()
{
    return _list.at( _current );
}

void Viewer::infoBoxMove()
{
    QPoint p = mapFromGlobal( QCursor::pos() );
    Options::Position oldPos = Options::instance()->infoBoxPosition();
    Options::Position pos = oldPos;
    int x = _display->mapFromParent( p ).x();
    int y = _display->mapFromParent( p ).y();
    int w = _display->width();
    int h = _display->height();

    if ( x < w/3 )  {
        if ( y < h/3  )
            pos = Options::TopLeft;
        else if ( y > h*2/3 )
            pos = Options::BottomLeft;
        else
            pos = Options::Left;
    }
    else if ( x > w*2/3 )  {
        if ( y < h/3  )
            pos = Options::TopRight;
        else if ( y > h*2/3 )
            pos = Options::BottomRight;
        else
            pos = Options::Right;
    }
    else {
        if ( y < h/3  )
            pos = Options::Top;
            else if ( y > h*2/3 )
                pos = Options::Bottom;
    }
    if ( pos != oldPos )  {
        Options::instance()->setInfoBoxPosition( pos );
        updateInfoBox();
    }
}

void Viewer::moveInfoBox()
{
    Options::Position pos = Options::instance()->infoBoxPosition();

    int lx = _display->pos().x();
    int ly = _display->pos().y();
    int lw = _display->width();
    int lh = _display->height();

    int bw = _infoBox->width();
    int bh = _infoBox->height();

    int bx, by;
    // x-coordinate
    if ( pos == Options::TopRight || pos == Options::BottomRight || pos == Options::Right )
        bx = lx+lw-5-bw;
    else if ( pos == Options::TopLeft || pos == Options::BottomLeft || pos == Options::Left )
        bx = lx+5;
    else
        bx = lx+lw/2-bw/2;


    // Y-coordinate
    if ( pos == Options::TopLeft || pos == Options::TopRight || pos == Options::Top )
        by = ly+5;
    else if ( pos == Options::BottomLeft || pos == Options::BottomRight || pos == Options::Bottom )
        by = ly+lh-5-bh;
    else
        by = ly+lh/2-bh/2;

    _infoBox->move(bx,by);
}

void Viewer::resizeEvent( QResizeEvent* e )
{
    moveInfoBox();
    QDialog::resizeEvent( e );
}

void Viewer::updateInfoBox()
{
    if ( currentInfo() ) {
        QMap<int, QPair<QString,QString> > map;
        QString text = Util::createInfoText( currentInfo(), &map );
        text = QString::fromLatin1("<qt>") + text + QString::fromLatin1("</qt>");
        _infoBox->setInfo( text, map );
        moveInfoBox();
    }
}

Viewer::~Viewer()
{
    if ( _latest == this )
        _latest = 0;
}

void Viewer::createToolBar()
{
    KIconLoader loader;
    KActionCollection* actions = new KActionCollection( this, "actions" );
    _toolbar = new KToolBar( this );
    DrawHandler* handler = _display->drawHandler();
    _select = new KToggleAction( i18n("Select"), loader.loadIcon(QString::fromLatin1("selecttool"), KIcon::Toolbar),
                         0, handler, SLOT( slotSelect() ),actions, "_select");
    _select->plug( _toolbar );
    _select->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _line = new KToggleAction( i18n("Line"), loader.loadIcon(QString::fromLatin1("linetool"), KIcon::Toolbar),
                         0, handler, SLOT( slotLine() ),actions, "_line");
    _line->plug( _toolbar );
    _line->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _rect = new KToggleAction( i18n("Rectangle"), loader.loadIcon(QString::fromLatin1("recttool"), KIcon::Toolbar),
                         0, handler, SLOT( slotRectangle() ),actions, "_rect");
    _rect->plug( _toolbar );
    _rect->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _circle = new KToggleAction( i18n("Circle"), loader.loadIcon(QString::fromLatin1("ellipsetool"), KIcon::Toolbar),
                           0, handler, SLOT( slotCircle() ),actions, "_circle");
    _circle->plug( _toolbar );
    _circle->setExclusiveGroup( QString::fromLatin1("ViewerTools") );

    _delete = KStdAction::cut( handler, SLOT( cut() ), actions, "cutAction" );
    _delete->plug( _toolbar );

    KAction* close = KStdAction::close( this,  SLOT( stopDraw() ),  actions,  "stopDraw" );
    close->plug( _toolbar );
}

void Viewer::toggleFullScreen()
{
    if ( !_showingFullScreen ) {

        KWin::Info info = KWin::info( winId() );
        _oldGeometry = info.frameGeometry;

        QRect r = QApplication::desktop()->screenGeometry( QApplication::desktop()->screenNumber( this ) );

        setFixedSize( r.size() );

        KWin::setType( winId(), NET::Override );
        KWin::setState( winId(), NET::StaysOnTop );

        setGeometry( r );
    }
    else {
        setMinimumSize(0,0);
        KWin::setType( winId(), NET::Normal );
        KWin::clearState( winId(), NET::StaysOnTop );
        setGeometry( _oldGeometry );
    }
    _showingFullScreen = !_showingFullScreen;
}

#include "viewer.moc"

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

Viewer* Viewer::_instance = 0;

Viewer* Viewer::instance( QWidget* parent )
{
    if ( !_instance )
        _instance = new Viewer( parent , "viewer" );
    return _instance;
}

Viewer::Viewer( QWidget* parent, const char* name )
    :KMainWindow( parent,  name ), _width(800), _height(600)
{
    _label = new DisplayArea( this );
    setCentralWidget( _label );
    setPaletteBackgroundColor( black );
    _label->setFixedSize( _width, _height );

    KIconLoader loader;
    _toolbar = new KToolBar( this );
    _select = new KToggleAction( i18n("Select"), loader.loadIcon(QString::fromLatin1("selecttool"), KIcon::Toolbar),
                         0, _label, SLOT( slotSelect() ),actionCollection(), "_select");
    _select->plug( _toolbar );
    _select->setExclusiveGroup( "ViewerTools" );

    _line = new KToggleAction( i18n("Line"), loader.loadIcon(QString::fromLatin1("linetool"), KIcon::Toolbar),
                         0, _label, SLOT( slotLine() ),actionCollection(), "_line");
    _line->plug( _toolbar );
    _line->setExclusiveGroup( "ViewerTools" );

    _rect = new KToggleAction( i18n("Rectangle"), loader.loadIcon(QString::fromLatin1("recttool"), KIcon::Toolbar),
                         0, _label, SLOT( slotRectangle() ),actionCollection(), "_rect");
    _rect->plug( _toolbar );
    _rect->setExclusiveGroup( "ViewerTools" );

    _circle = new KToggleAction( i18n("Circle"), loader.loadIcon(QString::fromLatin1("ellipsetool"), KIcon::Toolbar),
                           0, _label, SLOT( slotCircle() ),actionCollection(), "_circle");
    _circle->plug( _toolbar );
    _circle->setExclusiveGroup( "ViewerTools" );

    _delete = KStdAction::cut( _label, SLOT( cut() ), actionCollection(), "cutAction" );
    _delete->plug( _toolbar );

    KAction* close = KStdAction::close( this,  SLOT( stopDraw() ),  actionCollection(),  "stopDraw" );
    close->plug( _toolbar );

    _toolbar->hide();
    setupContextMenu();
}


void Viewer::setupContextMenu()
{
    _popup = new QPopupMenu( this );
    QAction* action;

    _firstAction = new QAction( "First", QIconSet(), "First", Key_Home, this );
    connect( _firstAction,  SIGNAL( activated() ), this, SLOT( showFirst() ) );
    _firstAction->addTo( _popup );

    _lastAction = new QAction( "Last", QIconSet(), "Last", Key_End, this );
    connect( _lastAction,  SIGNAL( activated() ), this, SLOT( showLast() ) );
    _lastAction->addTo( _popup );

    _nextAction = new QAction( "Show Next", QIconSet(), "Show Next", Key_PageDown, this );
    connect( _nextAction,  SIGNAL( activated() ), this, SLOT( showNext() ) );
    _nextAction->addTo( _popup );

    _prevAction = new QAction( "Show Previous",  QIconSet(), "Show Previous", Key_PageUp, this );
    connect( _prevAction,  SIGNAL( activated() ), this, SLOT( showPrev() ) );
    _prevAction->addTo( _popup );

    _popup->insertSeparator();

    action = new QAction( "Zoom in",  QIconSet(), "Zoom in", Key_Plus, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( zoomIn() ) );
    action->addTo( _popup );

    action = new QAction( "Zoom out",  QIconSet(), "Zoom out", Key_Minus, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( zoomOut() ) );
    action->addTo( _popup );

    _popup->insertSeparator();

    action = new QAction( "Rotate 90 degrees",  QIconSet(), "Rotate 90 degrees", Key_9, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( rotate90() ) );
    action->addTo( _popup );

    action = new QAction( "Rotate 180 degrees",  QIconSet(), "Rotate 180 degrees", Key_8, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( rotate180() ) );
    action->addTo( _popup );

    action = new QAction( "Rotate 270 degrees",  QIconSet(), "Rotate 270 degrees", Key_7, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( rotate270() ) );
    action->addTo( _popup );

    _popup->insertSeparator();

    action = new QAction( "Show Info Box", QIconSet(), "Show Info Box", Key_I, this, "showInfoBox", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowInfoBox( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showInfoBox() );

    action = new QAction( "Show Description", QIconSet(), "Show Description", Key_D, this, "showDescription", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDescription( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showDescription() );

    action = new QAction( "Show Time", QIconSet(), "Show Time", Key_T, this, "showTime", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDate( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showDate() );

    action = new QAction( "Show Names", QIconSet(), "Show Names", Key_N, this, "showNames", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowNames( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showNames() );

    action = new QAction( "Show Location", QIconSet(), "Show Location", Key_L, this, "showLocation", true );
    connect( action,  SIGNAL( toggled( bool ) ), this, SLOT( toggleShowLocation( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showLocation() );

    action = new QAction( "Show Keywords", QIconSet(), "Show Keywords", Key_K, this, "showKeywords", true );
    connect( action,  SIGNAL( toggled( bool ) ), this, SLOT( toggleShowKeyWords( bool ) ) );
    action->addTo( _popup );
    action->setOn( Options::instance()->showKeyWords() );

    _popup->insertSeparator();

    action = new QAction( "Draw On Image",  QIconSet(),  "Draw on Image",  0, this );
    connect( action,  SIGNAL( activated() ),  this, SLOT( startDraw() ) );
    action->addTo( _popup );

    action = new QAction( "Close",  QIconSet(), "Close", Key_Q, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( close() ) );
    action->addTo( _popup );
}

void Viewer::load( const ImageInfoList& list, int index )
{
    _list = list;
    _current = index;
    load();
}

void Viewer::pixmapLoaded( const QString&, int w, int h, int, const QPixmap& pixmap )
{
    // Erase
    QPainter p( _label );
    p.fillRect( 0, 0, _label->width(), _label->height(), paletteBackgroundColor() );
    w = QMIN( w, pixmap.width() );
    h = QMIN( h, pixmap.height() );
    _label->setFixedSize( w, h );
    _label->updateGeometry();

    _pixmap = pixmap;
    setDisplayedPixmap();
    _label->setDrawList( currentInfo()->drawList() );
}

void Viewer::load()
{
    QRect rect = QApplication::desktop()->screenGeometry( this );
    int w, h;

    if ( currentInfo()->angle() == 0 || currentInfo()->angle() == 180 )  {
        w = _width;
        h = _height;
    }
    else {
        h = _width;
        w = _height;
    }

    if ( w > rect.width() )  {
        h = (int) (h*((double)rect.width()/w));
        w = rect.width();
    }
    if ( h > rect.height() )  {
        w = (int) (w*((double)rect.height()/h));
        h = rect.height();
    }

    _label->setText( "Loading..." );

    ImageManager::instance()->load( currentInfo()->fileName( false ), this, currentInfo()->angle(), w,  h, false, true );
    _nextAction->setEnabled( _current +1 < (int) _list.count() );
    _prevAction->setEnabled( _current > 0 );
    _firstAction->setEnabled( _current > 0 );
    _lastAction->setEnabled( _current +1 < (int) _list.count() );
}

void Viewer::setDisplayedPixmap()
{
    QPixmap pixmap = _pixmap;
    if ( pixmap.isNull() )
        return;

    if ( Options::instance()->showInfoBox() )  {
        QPainter p( &pixmap );

        QString text = "" ;
        if ( Options::instance()->showDate() )  {
            text += "<b>Date:</b> ";
            if ( currentInfo()->startDate().isNull() )
                text += "Unknown";
            else if ( currentInfo()->endDate().isNull() )
                text += currentInfo()->startDate();
            else
                text += currentInfo()->startDate() + " to " + currentInfo()->endDate();
            text += "<br>";
        }

        // PENDING(blackie) The key is used both as a key and a label, which is a problem here.
        if ( Options::instance()->showLocation() )  {
            QString location = currentInfo()->optionValue( "Locations" ).join( ", " );
            if ( location )
                text += "<b>Location:</b> " + location + "<br>";
        }

        if ( Options::instance()->showNames() ) {
            QString persons = currentInfo()->optionValue( "Persons" ).join( ", " );
            if ( persons )
                text += "<b>Persons:</b> " + persons + "<br>";
        }

        if ( Options::instance()->showDescription() && !currentInfo()->description().isEmpty())  {
            if ( !text.isEmpty() )
                text += "<b>Description:</b> " +  currentInfo()->description() + "<br>";
        }

        if ( Options::instance()->showKeyWords() )  {
            QString keyWords = currentInfo()->optionValue( "Keywords" ).join( ", " );
            if ( keyWords )
                text += "<b>Key Words:</b> " + keyWords + "<br>";
        }

        Options::Position pos = Options::instance()->infoBoxPosition();
        if ( !text.isEmpty() )  {
            text = "<qt>" + text + "</qt>";

            QSimpleRichText txt( text, qApp->font() );

            if ( pos == Options::Top || pos == Options::Bottom )  {
                txt.setWidth( pixmap.width() - 20 ); // 2x5 pixels for inside border + 2x5 outside border.
            }
            else {
                int width = 25;
                do {
                    width += 25;
                    txt.setWidth( width );
                }  while ( txt.height() > width + 25 );
            }

            // -------------------------------------------------- Position rectangle
            QRect rect;
            rect.setWidth( txt.widthUsed() + 10 ); // 5 pixels border in all directions
            rect.setHeight( txt.height() + 10 );

            // x-coordinate
            if ( pos == Options::TopRight || pos == Options::BottomRight || pos == Options::Right )
                rect.moveRight( pixmap.width() - 5 );
            else if ( pos == Options::TopLeft || pos == Options::BottomLeft || pos == Options::Left )
                rect.moveLeft( 5 );
            else
                rect.moveLeft( (pixmap.width() - txt.widthUsed())/2+5 );

            // Y-coordinate
            if ( pos == Options::TopLeft || pos == Options::TopRight || pos == Options::Top )
                rect.moveTop( 5 );
            else if ( pos == Options::BottomLeft || pos == Options::BottomRight || pos == Options::Bottom )
                rect.moveBottom( pixmap.height() - 5 );
            else
                rect.moveTop( (pixmap.height()-txt.height())/2 + 5 );

            p.fillRect( rect, white );
            txt.draw( &p,  rect.left()+5, rect.top()+5,  QRect(),  _label->colorGroup());
            _textRect = rect;
        }
    }

    _label->setPixmap( pixmap );
}

void Viewer::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton )  {
        _moving = Options::instance()->showInfoBox() && _textRect.contains( _label->mapFromParent( e->pos() ) );
        _startPos = Options::instance()->infoBoxPosition();
        if ( _moving )
            _label->setCursor( PointingHandCursor );
    }
    else
        _moving = false;
    KMainWindow::mousePressEvent( e );
}

void Viewer::mouseMoveEvent( QMouseEvent* e )
{
    if ( !_moving )
        return;

    Options::Position oldPos = Options::instance()->infoBoxPosition();
    Options::Position pos = oldPos;
    int x = _label->mapFromParent( e->pos() ).x();
    int y = _label->mapFromParent( e->pos() ).y();
    int w = _label->width();
    int h = _label->height();

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
        setDisplayedPixmap();
    }
    KMainWindow::mouseMoveEvent( e );
}

void Viewer::mouseReleaseEvent( QMouseEvent* e )
{
    _label->setCursor( ArrowCursor  );
    if ( Options::instance()->infoBoxPosition() != _startPos )
        Options::instance()->save();
    KMainWindow::mouseReleaseEvent( e );
}

void Viewer::contextMenuEvent( QContextMenuEvent * e )
{
    _popup->exec( e->globalPos() );
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

void Viewer::zoomIn()
{
    _width = _width*4/3;
    _height = _height*4/3;
    resize( _width, _height );
    load();
}

void Viewer::zoomOut()
{
    _width = _width*3/4;
    _height = _height*3/4;
    _label->setMinimumSize(0,0);
    resize( _width, _height );
    load();
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
    setDisplayedPixmap();
    Options::instance()->save();
}

void Viewer::toggleShowDescription( bool b )
{
    Options::instance()->setShowDescription( b );
    setDisplayedPixmap();
    Options::instance()->save();
}

void Viewer::toggleShowDate( bool b )
{
    Options::instance()->setShowDate( b );
    setDisplayedPixmap();
    Options::instance()->save();
}

void Viewer::toggleShowNames( bool b )
{
    Options::instance()->setShowNames( b );
    setDisplayedPixmap();
    Options::instance()->save();
}

void Viewer::toggleShowLocation( bool b )
{
    Options::instance()->setShowLocation( b );
    setDisplayedPixmap();
    Options::instance()->save();
}

void Viewer::toggleShowKeyWords( bool b )
{
    Options::instance()->setShowKeyWords( b );
    setDisplayedPixmap();
    Options::instance()->save();
}


Viewer::~Viewer()
{
    _instance = 0;
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
    currentInfo()->setDrawList( _label->drawList() );
}

void Viewer::startDraw()
{
    _label->slotSelect();
    _toolbar->show();
}

void Viewer::stopDraw()
{
    _label->stopDrawings();
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

#include "viewer.moc"

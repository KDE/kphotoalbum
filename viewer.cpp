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

Viewer* Viewer::_instance = 0;

Viewer* Viewer::instance()
{
    if ( !_instance )
        _instance = new Viewer( 0, "viewer" );
    return _instance;
}

Viewer::Viewer( QWidget* parent, const char* name )
    :QDialog( parent,  name ),  _pos( BottomRight ), _showInfoBox(true), _showDescription(true),
     _showDate(true), _showNames(false), _showLocation(false)
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setResizeMode( QLayout::FreeResize );
    _label = new QLabel( this );
    _label->setAlignment( AlignCenter );
    layout->addWidget( _label );
    _label->setBackgroundMode( NoBackground );

    _popup = new QPopupMenu( this );

    QAction* action;

    action = new QAction( "Show Next", QIconSet(), "Show Next", Key_PageDown, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( showNext() ) );
    action->addTo( _popup );

    action = new QAction( "Show Previous",  QIconSet(), "Show Previous", Key_PageUp, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( showPrev() ) );
    action->addTo( _popup );

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
    action->setOn( _showInfoBox );

    action = new QAction( "Show Description", QIconSet(), "Show Description", Key_D, this, "showDescription", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDescription( bool ) ) );
    action->addTo( _popup );
    action->setOn( _showDescription );

    action = new QAction( "Show Time", QIconSet(), "Show Time", Key_T, this, "showTime", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowDate( bool ) ) );
    action->addTo( _popup );
    action->setOn( _showDate );

    action = new QAction( "Show Names", QIconSet(), "Show Names", Key_N, this, "showNames", true );
    connect( action, SIGNAL( toggled( bool ) ), this, SLOT( toggleShowNames( bool ) ) );
    action->addTo( _popup );
    action->setOn( _showNames );

    action = new QAction( "Show Location", QIconSet(), "Show Location", Key_L, this, "showLocation", true );
    connect( action,  SIGNAL( toggled( bool ) ), this, SLOT( toggleShowLocation( bool ) ) );
    action->addTo( _popup );
    action->setOn( _showLocation );

    _popup->insertSeparator();

    action = new QAction( "Close",  QIconSet(), "Close", 0, this );
    connect( action,  SIGNAL( activated() ), this, SLOT( close() ) );
    action->addTo( _popup );
}

void Viewer::load( const ImageInfoList& list, int index )
{
    _list = list;
    _current = index;
    _info = *( _list.at( _current ) );
    load();
}

void Viewer::pixmapLoaded( const QString&, int, int, int, const QPixmap& pixmap )
{
    // Erase
    QPainter p( _label );
    p.fillRect( 0, 0, _label->width(), _label->height(), paletteBackgroundColor() );

    _pixmap = pixmap;
    setDisplayedPixmap();
}

void Viewer::show()
{
    resize( 800, 600 );
    QDialog::show();
}

void Viewer::load()
{
    // Erase
    QPainter p( _label );
    p.fillRect( 0, 0, _label->width(), _label->height(), paletteBackgroundColor() );

    _label->setText( "Loading..." );
    ImageManager::instance()->load( _info.fileName( false ), this, _info.angle(), _label->width(),  _label->height(), false );
}

void Viewer::setDisplayedPixmap()
{

    QPixmap pixmap = _pixmap;
    if ( pixmap.isNull() )
        return;

    if ( _showInfoBox )  {
        QPainter p( &pixmap );

        QString text = "" ;
        if ( _showDate )  {
            text += "<b>Date:</b> ";
            if ( _info.startDate().isNull() )
                text += "Unknown";
            else if ( _info.endDate().isNull() )
                text += _info.startDate();
            else
                text += _info.startDate() + " to " + _info.endDate();
            text += "<br>";
        }

        // PENDING(blackie) The key is used both as a key and a label, which is a problem here.
        if ( _showLocation )  {
            QString location = _info.optionValue( "Locations" ).join( ", " );
            if ( location )
                text += "<b>Location:</b> " + location + "<br>";
        }

        if ( _showNames ) {
            QString persons = _info.optionValue( "Persons" ).join( ", " );
            if ( persons )
                text += "<b>Persons:</b> " + persons + "<br>";
        }

        if ( _showDescription && !_info.description().isEmpty())  {
            if ( !text.isEmpty() )
                text += "<b>Description:</b> ";

            text += _info.description();
        }

        if ( !text.isEmpty() )  {
            text = "<qt>" + text + "</qt>";

            QSimpleRichText txt( text, qApp->font() );

            if ( _pos == Top || _pos == Bottom )  {
                txt.setWidth( _label->width() - 20 ); // 2x5 pixels for inside border + 2x5 outside border.
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
            if ( _pos == TopRight || _pos == BottomRight || _pos == Right )
                rect.moveRight( _label->width() - 5 );
            else if ( _pos == TopLeft || _pos == BottomLeft || _pos == Left )
                rect.moveLeft( 5 );
            else
                rect.moveLeft( (_label->width() - txt.widthUsed())/2+5 );

            // Y-coordinate
            if ( _pos == TopLeft || _pos == TopRight || _pos == Top )
                rect.moveTop( 5 );
            else if ( _pos == BottomLeft || _pos == BottomRight || _pos == Bottom )
                rect.moveBottom( _label->height() - 5 );
            else
                rect.moveTop( (_label->height()-txt.height())/2 + 5 );

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
        _moving = _showInfoBox && _textRect.contains( e->pos() );
        if ( _moving )
            _label->setCursor( PointingHandCursor );
    }
    else
        _moving = false;
    QDialog::mousePressEvent( e );
}

void Viewer::mouseMoveEvent( QMouseEvent* e )
{
    if ( !_moving )
        return;

    Position oldPos = _pos;
    int x = e->pos().x();
    int y = e->pos().y();
    int w = width();
    int h = height();

    if ( x < w/3 )  {
        if ( y < h/3  )
            _pos = TopLeft;
        else if ( y > h*2/3 )
            _pos = BottomLeft;
        else
            _pos = Left;
    }
    else if ( x > w*2/3 )  {
        if ( y < h/3  )
            _pos = TopRight;
        else if ( y > h*2/3 )
            _pos = BottomRight;
        else
            _pos = Right;
    }
    else {
        if ( y < h/3  )
            _pos = Top;
        else if ( y > h*2/3 )
            _pos = Bottom;
    }
    if ( _pos != oldPos )
        setDisplayedPixmap();
    QDialog::mouseMoveEvent( e );
}

void Viewer::mouseReleaseEvent( QMouseEvent* e )
{
    _label->setCursor( ArrowCursor  );
    QDialog::mouseReleaseEvent( e );
}

void Viewer::contextMenuEvent( QContextMenuEvent * e )
{
    _popup->exec( e->globalPos() );
}

void Viewer::showNext()
{
    if ( _current +1 < (int) _list.count() )  {
        _current++;
        load();
    }
}

void Viewer::showPrev()
{
    if ( _current > 0  )  {
        _current--;
        load();
    }
}

void Viewer::zoomIn()
{
    resize( width()*4/3,  height()*4/3 );
    load();
}

void Viewer::zoomOut()
{
    resize( width()*3/4,  height()*3/4 );
    load();
}

void Viewer::rotate90()
{
    _info.rotate( 90 );
    load();
}

void Viewer::rotate180()
{
    _info.rotate( 180 );
    load();
}

void Viewer::rotate270()
{
    _info.rotate( 270 );
    load();
}

void Viewer::toggleShowInfoBox( bool b )
{
    _showInfoBox = b;
    setDisplayedPixmap();
}

void Viewer::toggleShowDescription( bool b )
{
    _showDescription = b;
    setDisplayedPixmap();
}

void Viewer::toggleShowDate( bool b )
{
    _showDate = b;
    setDisplayedPixmap();
}

void Viewer::toggleShowNames( bool b )
{
    _showNames = b;
    setDisplayedPixmap();
}

void Viewer::toggleShowLocation( bool b )
{
    _showLocation = b;
    setDisplayedPixmap();
}



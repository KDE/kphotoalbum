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
    :QDialog( parent,  name ),  _pos( BottomRight )
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

    action = new QAction( "    Zoom out",  QIconSet(), "Zoom out", Key_Minus, this );
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
    _label->setText( "Loading..." );
    ImageManager::instance()->load( _info.fileName( false ), this, _info.angle(), _label->width(),  _label->height(), false );
}

void Viewer::setDisplayedPixmap()
{
    QPixmap pixmap = _pixmap;
    QPainter p( &pixmap );
    QSimpleRichText txt( _info.description(), qApp->font() );

    if ( _pos == Top || _pos == Bottom )  {
        txt.setWidth( _label->width() - 20 ); // 2x5 pixels for inside border + 2x5 outside border.
    }
    else {
        int width = 50;
        do {
            width += 50;
            txt.setWidth( width );
        }  while ( txt.height() > width + 50 );
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
    _label->setPixmap( pixmap );
    _textRect = rect;
}

void Viewer::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == LeftButton )  {
        _moving = _textRect.contains( e->pos() );
        if ( _moving )
            _label->setCursor( PointingHandCursor );
    }
    else
        _moving = false;
    QDialog::mousePressEvent( e );
}

void Viewer::mouseMoveEvent( QMouseEvent* e )
{
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
    setCursor( ArrowCursor  );
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

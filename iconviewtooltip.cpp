/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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

#include "iconviewtooltip.h"
#include <qcursor.h>
#include "thumbnail.h"
#include <qlayout.h>
#include "util.h"
#include <qtooltip.h>
#include "options.h"
#include <qmime.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include "imagemanager.h"

/**
   This class takes care of showing tooltips for the individual items in the iconview.
   I tried implementing this with QToolTip::maybeTip() on the iconview, but it had two
   disadvantages: (1) When scrolling using the scrollbutton of the mouse,
   the tooltip was not updated. (2) Either the tooltip would not follow the
   mouse( and would therefore stand on top of the image), or it flickered.
*/
IconViewToolTip::IconViewToolTip( QIconView* view, const char* name )
    : QLabel( view, name, WStyle_Customize | WStyle_NoBorder | WType_TopLevel | WX11BypassWM | WStyle_Tool ), _view( view ), _current(0),
      _widthInverse( false ), _heightInverse( false )
{
	setAlignment( AlignAuto | AlignTop );
    setFrameStyle( QFrame::Box | QFrame::Plain );
    setLineWidth(1);
    setMargin(1);
    setPalette( QToolTip::palette() );
}

bool IconViewToolTip::eventFilter( QObject* o , QEvent* event )
{
    if ( o == _view->viewport() && event->type() == QEvent::Leave )
        hide();

    else if ( event->type() == QEvent::MouseMove )
        showToolTips( false );
    return false;
}

void IconViewToolTip::showToolTips( bool force )
{
    QIconViewItem* item = itemAtCursor();
    if ( item == 0 )
        return;

    if ( force || (item && item != _current) ) {
        ThumbNail* tn = static_cast<ThumbNail*>( item );
        if ( loadImage( *tn->imageInfo() ) ) {
            setText( QString::null );
            setText( QString::fromLatin1("<qt><table cols=\"2\"><tr><td><img src=\"%1\"></td><td>%2</td></tr></qt>")
                     .arg(tn->fileName()).
                     arg(Util::createInfoText( tn->imageInfo(), 0 ) ) );
        }

        _current = item;
        resize( sizeHint() );
        _view->setFocus();
    }

    placeWindow();
    show();
}


QIconViewItem* IconViewToolTip::itemAtCursor()
{
    QPoint pos = QCursor::pos();
    int x,y;
    pos = _view->mapFromGlobal( pos );
    _view->viewportToContents( pos.x(), pos.y(), x, y );
    QIconViewItem* item = _view->findItem( QPoint(x,y) );
    return item;
}

void IconViewToolTip::setActive( bool b )
{
    if ( b ) {
        showToolTips(true);
        _view->viewport()->installEventFilter( this );
        show();
    }
    else {
        _view->viewport()->removeEventFilter( this );
        hide();
    }
}

void IconViewToolTip::placeWindow()
{
    // First try to set the position.
    QPoint pos = QCursor::pos() + QPoint( 20, 20 );
    if ( _widthInverse )
        pos.setX( pos.x() - 30 - width() );
    if ( _heightInverse )
        pos.setY( pos.y() - 30 - height() );

    QRect geom = qApp->desktop()->screenGeometry( QCursor::pos() );

    // Now test whether the window moved outside the screen
    if ( _widthInverse ) {
        if ( pos.x() <  geom.x() ) {
            pos.setX( QCursor::pos().x() + 20 );
            _widthInverse = false;
        }
    }
    else {
        if ( pos.x() + width() > geom.right() ) {
            pos.setX( QCursor::pos().x() - width() );
            _widthInverse = true;
        }
    }

    if ( _heightInverse ) {
        if ( pos.y() < geom.y()  ) {
            pos.setY( QCursor::pos().y() + 10 );
            _heightInverse = false;
        }
    }
    else {
        if ( pos.y() + height() > geom.bottom() ) {
            pos.setY( QCursor::pos().y() - 10 - height() );
            _heightInverse = true;
        }
    }

    move( pos );

}

void IconViewToolTip::clear()
{
    // I can't find any better way to remove the images from the cache.
    for( QStringList::Iterator it = _loadedImages.begin(); it != _loadedImages.end(); ++it ) {
        QMimeSourceFactory::defaultFactory()->setImage( *it, QImage() );
    }
    _loadedImages.clear();
}


bool IconViewToolTip::loadImage( const ImageInfo& info )
{
    _currentFileName = info.fileName();
    if ( !_loadedImages.contains( info.fileName() ) ) {
        ImageManager::instance()->load( info.fileName(),  this, info.angle(), 256, 256, true, true );
        QMimeSourceFactory::defaultFactory()->setImage( info.fileName(), QImage() );
        _loadedImages.append( info.fileName() );
        return false;
    }
    return true;
}

void IconViewToolTip::pixmapLoaded( const QString& fileName, int /*width*/, int /*height*/, int /*angle*/, const QImage& image )
{
    QMimeSourceFactory::defaultFactory()->setImage( fileName, image );
    if ( fileName == _currentFileName )
        showToolTips(true);
}

#include "iconviewtooltip.moc"

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

#include "ThumbnailToolTip.h"
#include <qcursor.h>
//Added by qt3to4:
#include <QEvent>
#include <QLabel>
#include <Q3Frame>
#include "Utilities/Util.h"
#include <qtooltip.h>
#include "Settings/SettingsData.h"
#include <qmime.h>
#include <qapplication.h>
#include "ImageManager/Manager.h"
#include "DB/ImageInfo.h"
#include "ThumbnailWidget.h"
#include "DB/ImageDB.h"
#include <kdebug.h>

/**
   \class ThumbnailToolTip
   This class takes care of showing tooltips for the individual items in the thumbnail view.
   I tried implementing this with QToolTip::maybeTip() on the iconview, but it had the
   disadvantages that either the tooltip would not follow the
   mouse( and would therefore stand on top of the image), or it flickered.
*/

ThumbnailView::ThumbnailToolTip::ThumbnailToolTip( ThumbnailWidget* view )
    : QLabel( view, Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WType_TopLevel
#ifdef TEMPORARILY_REMOVED
              | WX11BypassWM
#endif
              | Qt::WStyle_Tool ), _view( view ),
      _widthInverse( false ), _heightInverse( false )
{
#ifdef TEMPORARILY_REMOVED
    setAlignment( AlignAuto | Qt::AlignTop );
#endif
    setFrameStyle( Q3Frame::Box | Q3Frame::Plain );
    setLineWidth(1);
    setMargin(1);
    setPalette( QToolTip::palette() );
}

bool ThumbnailView::ThumbnailToolTip::eventFilter( QObject* o , QEvent* event )
{
    if ( o == _view->viewport() && event->type() == QEvent::Leave )
        hide();

    else if ( event->type() == QEvent::MouseMove )
        showToolTips( false );
    return false;
}

void ThumbnailView::ThumbnailToolTip::showToolTips( bool force )
{
    QString fileName = _view->fileNameUnderCursor();
    if ( fileName.isNull() )
        return;

    if ( force || (fileName != _currentFileName) ) {
        if ( loadImage( fileName ) ) {
            setText( QString::null );
            int size = Settings::SettingsData::instance()->previewSize();
            if ( size != 0 ) {
                setText( QString::fromLatin1("<p><table cols=\"2\"><tr><td><img src=\"%1\"></td><td>%2</td></tr></p>")
                         .arg(fileName).
                         arg(Utilities::createInfoText( DB::ImageDB::instance()->info( fileName ), 0 ) ) );
            }
            else {
                setText( QString::fromLatin1("<p>%1</p>").arg( Utilities::createInfoText( DB::ImageDB::instance()->info( fileName ), 0 ) ) );
            }
        }

        _currentFileName = fileName;
        resize( sizeHint() );
        _view->setFocus();
    }

    placeWindow();
    show();
}


void ThumbnailView::ThumbnailToolTip::setActive( bool b )
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

void ThumbnailView::ThumbnailToolTip::placeWindow()
{
#ifdef TEMPORARILY_REMOVED
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

#else
    kDebug() << "TEMPORARILY REMOVED: " ;
#endif
}

void ThumbnailView::ThumbnailToolTip::clear()
{
#ifdef TEMPORARILY_REMOVED
// I can't find any better way to remove the images from the cache.
    for( QStringList::Iterator it = _loadedImages.begin(); it != _loadedImages.end(); ++it ) {
        Q3MimeSourceFactory::defaultFactory()->setImage( *it, QImage() );
    }
    _loadedImages.clear();
#else
    kDebug() << "TEMPORARILY REMOVED: " ;
#endif
}


bool ThumbnailView::ThumbnailToolTip::loadImage( const QString& fileName )
{
#ifdef TEMPORARILY_REMOVED
    int size = Settings::SettingsData::instance()->previewSize();
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( fileName );
    if ( size != 0 ) {
        if ( !_loadedImages.contains( fileName ) ) {
            ImageManager::ImageRequest* request = new ImageManager::ImageRequest( fileName, QSize( size, size ), info->angle(), this );
            request->setCache();
            request->setPriority();
            ImageManager::Manager::instance()->load( request );
            Q3MimeSourceFactory::defaultFactory()->setImage( fileName, QImage() );
            _loadedImages.append( fileName );
            return false;
        }
    }
    return true;
#else
    kDebug() << "TEMPORARILY REMOVED: " ;
    Q_UNUSED(fileName);
    return false;
#endif
}

void ThumbnailView::ThumbnailToolTip::pixmapLoaded( const QString& fileName, const QSize& /*size*/,
                                    const QSize& /*fullSize*/, int /*angle*/, const QImage& image, bool /*loadedOK*/ )
{
#ifdef TEMPORARILY_REMOVED
    Q3MimeSourceFactory::defaultFactory()->setImage( fileName, image );
    if ( fileName == _currentFileName )
        showToolTips(true);
#else
    kDebug() << "TEMPORARILY REMOVED: " ;
    Q_UNUSED( fileName );
    Q_UNUSED( image );
#endif
}

#include "ThumbnailToolTip.moc"

#include "MPlayer.h"
#include <kprocess.h>
#include <qstringlist.h>
#include <qwidget.h>
#include <qimage.h>
#include <qdir.h>

using namespace Video;

/*
 * Some hints for me :-)
 *
 * mplayer -vo jpeg:maxfiles=1000:outdir=abc mvi_0416.avi
 *
 * mplayer -vf rotate mvi_0471.avi (rotate 90 degrees)
 *
 * -ao null disables audio
 *
 * mplayer -frames 10 -ss 12 -sstep 20 -vo jpeg:outdir=abc mvi_0417.avi dump 10 frames with 20 secs in between starting at sec 12
 *
 *
 */
void Video::MPlayer::play( const QStringList& movies )
{
    _pendingMovies = movies;

    if ( _process->isRunning() )
        _process->kill();
    else
        startMovie();
}

Video::MPlayer::MPlayer()
{
    _process = new KProcess;
    connect( _process, SIGNAL( processExited(KProcess *) ), this, SLOT( startMovie() ) );

}

void Video::MPlayer::startMovie()
{
    emit playing( false );

    if ( _pendingMovies.empty() )
        return;

#ifdef TEMPORARILY_REMOVED
    QWidget* widget = new QWidget;
    widget->setBackgroundMode( NoBackground );
    widget->resize( 640,480 );
    widget->show();
    int wid = widget->winId();
#endif

    // PENDING(blackie) VIDEO allow for different players.
    _process->clearArguments();
    *_process << QString::fromLatin1( "mplayer" ) << QString::fromLatin1( "-fs" );

    //          << QString::fromLatin1("-vo") << QString::fromLatin1( "x11" );
    //<< QString::fromLatin1( "-wid" ) << QString::number( wid );

    for( QStringList::ConstIterator movieIt = _pendingMovies.begin(); movieIt != _pendingMovies.end(); ++movieIt ) {
        *_process << *movieIt;
    }
    _process->start();
    emit playing( true );

    _pendingMovies.clear();
}

void Video::MPlayer::loadSnapshot( ImageManager::ImageRequest* request )
{
    _snapshot.load( request );
}


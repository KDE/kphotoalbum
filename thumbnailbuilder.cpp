#include "thumbnailbuilder.h"
#include "imagemanager.h"
#include "imagedb.h"
#include <klocale.h>
#include "options.h"
#include <qimage.h>

ThumbnailBuilder::ThumbnailBuilder( QWidget* parent, const char* name )
    :QProgressDialog( parent, name )
{
    _images = ImageDB::instance()->images();
    setTotalSteps( _images.count() );
    setProgress( 0 );
    setLabelText( i18n("Generating thumbnails") );
    _index = 0;
    generateNext();
}

void ThumbnailBuilder::generateNext()
{
    ImageInfo* info = _images.at(_index);
    ++_index;
    setProgress( _index );
    int size = Options::instance()->thumbSize();
    ImageManager::instance()->load( info->fileName(),  this, info->angle(), size, size, true, true );
}

void ThumbnailBuilder::pixmapLoaded( const QString&, const QSize& /*size*/, const QSize& /*fullSize*/, int, const QImage& )
{
    if ( wasCanceled() )
        delete this;
    else if ( _index == _images.count() )
        delete this;
    else
        generateNext();
}

#include "thumbnailbuilder.moc"

#include "imagecounter.h"

ImageCounter::ImageCounter( QWidget* parent, const char* name )
    :QLabel( parent, name ), _partial(0), _total(0)
{
    setFrameStyle( Sunken );
    setMargin(5);
}

void ImageCounter::setPartial( int c )
{
    _partial = c;
    updateText();
}

void ImageCounter::setTotal( int c )
{
    _total = c;
    updateText();
}

void ImageCounter::updateText()
{
    setText( QString::fromLatin1( "Showing %1 of %2 images" ).arg( _partial ).arg( _total ) );
}

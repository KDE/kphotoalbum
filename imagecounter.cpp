#include "imagecounter.h"
#include <klocale.h>
#include <qlayout.h>

ImageCounter::ImageCounter( QWidget* parent, const char* name )
    :QLabel( parent, name )
{
    setText( QString::fromLatin1( "---" ) );
    setMargin( 5 );
}

void ImageCounter::setMatchCount( int start, int end, int matches )
{
    if (start == -1 )
        setText( i18n( "Showing %1 images" ).arg( matches ) );
    else
        setText( i18n( "Showing %1-%2 of %3").arg(start).arg(end).arg(matches) );
}

void ImageCounter::setTotal( int c )
{
    setText( i18n( "Total: %1" ).arg(c) );
}

void ImageCounter::showingOverview()
{
    setText( QString::fromLatin1( "---" ) );
}

#include "imagecounter.moc"

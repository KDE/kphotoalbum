#include "imagefolder.h"
#include <klocale.h>
#include "imagedb.h"
#include "imagesearchinfo.h"
#include "options.h"
#include <kstandarddirs.h>
ImageFolder::ImageFolder( const ImageSearchInfo& info, Browser* parent )
    :Folder( info, parent ), _from(-1), _to(-1)
{
    setText( i18n( "View Images" ) );
    setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/imagesIcon.png") ) );
}

ImageFolder::ImageFolder( const ImageSearchInfo& info, int from, int to, Browser* parent )
    :Folder(info,parent), _from( from ), _to( to )
{
    setText( i18n( "View Images (%1-%2)").arg(from).arg(to) );
    setPixmap( locate("data", QString::fromLatin1("kimdaba/pics/imagesIcon.png") ) );
}

void ImageFolderAction::action()
{
    ImageDB::instance()->search( _info, _from, _to );

    if ( _addExtraToBrowser ) {
        // Add all the following image fractions to the image list, so the user
        // simply cann use the forward button to see the following images.
        int count = ImageDB::instance()->count( _info );
        int maxPerPage = Options::instance()->maxImages();

        if ( count > maxPerPage ) {
            int last = _to;
            while ( last < count ) {
                ImageFolderAction* action =
                    new ImageFolderAction( _info, last, QMIN( count, last+maxPerPage-1 ), _browser );

                // We do not want this new action to create extra items as we do here.
                action->_addExtraToBrowser = false;

                _browser->_list.append( action );
                _browser->emitSignals();
                last += maxPerPage;
            }
        }

        // Only add extra items the first time the action is executed.
        _addExtraToBrowser = false;
    }
}

FolderAction* ImageFolder::action( bool /* ctrlDown */ )
{
    return new ImageFolderAction( _info, _from, _to, _browser );
}

ImageFolderAction::ImageFolderAction( const ImageSearchInfo& info, int from, int to,  Browser* browser )
    : FolderAction( info, browser ), _from(from), _to(to), _addExtraToBrowser( true )
{
}

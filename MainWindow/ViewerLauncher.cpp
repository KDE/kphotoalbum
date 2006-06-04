#include "ViewerLauncher.h"
#include <qmessagebox.h>
#include <klocale.h>
#include "Utilities/Util.h"
#include "MainWindow/Window.h"
#include "Viewer/ViewerWidget.h"
#include "Video/Player.h"

void MainWindow::ViewerLauncher::launch( const QStringList& files, bool reuse, bool slideShow, bool random )
{
    if ( files.count() == 0 ) {
        QMessageBox::warning( MainWindow::Window::theMainWindow(), i18n("No Images to Display"),
                              i18n("None of the selected images were available on the disk.") );
        return;
    }

    QStringList images;
    QStringList movies;

    for( QStringList::ConstIterator it = files.begin(); it != files.end(); ++it ) {
        if ( Utilities::isMovie( *it ) )
            movies.append( *it );
        else
            images.append( *it );
    }

    if ( movies.count() >= images.count() )
        Video::Player::instance()->play( movies );
    else
        launchImages( images, reuse, slideShow, random );
}

void MainWindow::ViewerLauncher::launchImages(QStringList images, bool reuse, bool slideShow, bool random )
{
    if (random)
        images = Utilities::shuffle( images );

    Viewer::ViewerWidget* viewer;
    if ( reuse && Viewer::ViewerWidget::latest() ) {
        viewer = Viewer::ViewerWidget::latest();
        viewer->raise();
        viewer->setActiveWindow();
    }
    else {
        viewer = new Viewer::ViewerWidget( "viewer" );
        QObject::connect( viewer, SIGNAL( dirty() ), MainWindow::Window::theMainWindow(), SLOT( markDirty() ) );
    }
    viewer->show( slideShow );

    viewer->load( images );
    viewer->raise();
}


#include <qapplication.h>
#include "thumbnailview.h"
#include "options.h"
#include <qdir.h>
#include "mainview.h"

int main( int argc, char** argv ) {
    QApplication app( argc, argv );

    MainView* view = new MainView( 0,  "view" );
    view->resize(800, 600);
    view->show();

    QObject::connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
    return app.exec();
}

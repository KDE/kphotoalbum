#include "thumbnailview.h"
#include "options.h"
#include <qdir.h>
#include "mainview.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

static const KCmdLineOptions options[] =
{
	{ "c ", I18N_NOOP("Config file"), 0 },
	{ 0, 0, 0}
};

int main( int argc, char** argv ) {
    KAboutData aboutData( "kimdaba", I18N_NOOP("KimDaba"), "0.01",
                          I18N_NOOP("KDE Image Database"), KAboutData::License_GPL );
    aboutData.addAuthor( "Jesper K. Pedersen", "Development", "blackie@kde.org" );

    KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->isSet( "c" ) )
        Options::setConfFile( args->getOption( "c" ) );

    MainView* view = new MainView( 0,  "view" );
    qApp->setMainWidget( view );
    view->resize(800, 600);
    view->show();

    return app.exec();
}

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

#include "thumbnailview.h"
#include "options.h"
#include <qdir.h>
#include "mainview.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kimageio.h>
#include "mysplashscreen.h"

static const KCmdLineOptions options[] =
{
	{ "c ", I18N_NOOP("Config file"), 0 },
    { "demo", I18N_NOOP( "Starts KimDaBa with a prebuilt set of demo images" ), 0 },
    { "import ", I18N_NOOP( "Import file" ), 0 },
	{ 0, 0, 0}
};

int main( int argc, char** argv ) {
    KAboutData aboutData( "kimdaba", I18N_NOOP("KimDaBa"), "1.2",
                          I18N_NOOP("KDE Image Database"), KAboutData::License_GPL );
    aboutData.addAuthor( "Jesper K. Pedersen", I18N_NOOP("Development"), "blackie@kde.org" );

    aboutData.addCredit( "Will Stephenson", "Developing an Icon for KimDaBa", "will@stevello.free-online.co.uk" );
    aboutData.addCredit( "Jozef Riha","Testing early versions of KimDaBa.", "zefo@seznam.cz" );
    aboutData.addCredit( "Teemu Rytilahti",
                         "Sending patches implementing (.) the \"Set As Wallpaper\" menu in the viewer."
                         "(.) Theme support for HTML generation", "teemu.rytilahti@kde-fi.org" );
    aboutData.addCredit( "Reimar Imhof", "Patch to sort items in option listboxes", "Reimar.Imhof@netCologne.de" );
    aboutData.addCredit( "Thomas Schwarzgruber", "Patch to sort images in the thumbnail view, plus reading time info out of EXIF images for existing images", "possebaer@gmx.at" );
    aboutData.addCredit( "Marcel Wiesweg", "Patch which speed up loading of thumbnails plus preview in image property dialog.", "marcel.wiesweg@gmx.de" );
    aboutData.addCredit( "Marco Caldarelli", "Patch for making it possible to reread EXIF info using a nice dialog.", "caldarel@yahoo.it" );

    KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    MySplashScreen* splash = new MySplashScreen();
    splash->show();

    KImageIO::registerFormats();
    MainView* view = new MainView( 0, "view" );

    qApp->setMainWidget( view );
    view->resize(800, 600);
    splash->finish( view );

    int code = app.exec();

    // I need this to ensure that I don't get a crash on exit from the kate part.
    delete view;
    return code;
}

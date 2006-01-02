/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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

#include "options.h"
#include <qdir.h>
#include "mainview.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kimageio.h>
#include "mysplashscreen.h"
#include <qdatetime.h>

static const KCmdLineOptions options[] =
{
	{ "c ", I18N_NOOP("Config file"), 0 },
        { "demo", I18N_NOOP( "Starts KimDaBa with a prebuilt set of demo images" ), 0 },
        { "import ", I18N_NOOP( "Import file" ), 0 },
        { "export-in-2.1-format", I18N_NOOP( "This will make an attempt at saving in a format understandable by KimDaBa 2.1" ), 0 },
	{ 0, 0, 0}
};

int main( int argc, char** argv ) {
    KAboutData aboutData( "kimdaba", I18N_NOOP("KimDaBa"), "SVN",
                          I18N_NOOP("KDE Image Database"), KAboutData::License_GPL,
                          0, 0, "http://ktown.kde.org/kimdaba");
    aboutData.addAuthor( "Jesper K. Pedersen", I18N_NOOP("Development"), "blackie@kde.org" );

    aboutData.addCredit( "Will Stephenson", "Developing an Icon for KimDaBa", "will@stevello.free-online.co.uk" );
    aboutData.addCredit( "Teemu Rytilahti",
                         "Sending patches implementing (.) the \"Set As Wallpaper\" menu in the viewer."
                         "(.) Theme support for HTML generation", "teemu.rytilahti@kde-fi.org" );
    aboutData.addCredit( "Reimar Imhof", "Patch to sort items in option listboxes", "Reimar.Imhof@netCologne.de" );
    aboutData.addCredit( "Thomas Schwarzgruber", "Patch to sort images in the thumbnail view, plus reading time info out of EXIF images for existing images", "possebaer@gmx.at" );
    aboutData.addCredit( "Marcel Wiesweg", "Patch which speed up loading of thumbnails plus preview in image property dialog.", "marcel.wiesweg@gmx.de" );
    aboutData.addCredit( "Marco Caldarelli", "Patch for making it possible to reread EXIF info using a nice dialog.", "caldarel@yahoo.it" );
    aboutData.addCredit( "Jean-Michel FAYARD", "(.) Patch with directory info made available through the browser. (.) Patch for adding a check box for \"and/or\" searches in the search page.", "jmfayard@gmail.com" );
    aboutData.addCredit( "Robert L Krawitz", "Numerous patches plus profiling KimDaBa again and again.", "rlk@alum.mit.edu" );

    aboutData.setHomepage( "http://ktown.kde.org/kimdaba/" );

    KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    MySplashScreen* splash = new MySplashScreen();
    splash->show();

    KImageIO::registerFormats();
    MainView* view = new MainView( 0, "view" );

    // qApp->setMainWidget( view );
    view->setGeometry( Options::instance()->windowGeometry( Options::MainWindow ) );

    int code = app.exec();

    return code;
}

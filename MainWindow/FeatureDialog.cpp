/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
#include "FeatureDialog.h"
#include <config.h>
#include <klocale.h>
#include <qlayout.h>
#include <kapplication.h>
#include "Exif/Database.h"
#include <kuserprofile.h>
#include <ImageManager/VideoManager.h>
#include <kmediaplayer/player.h>
#include <kparts/componentfactory.h>

using namespace MainWindow;

FeatureDialog::FeatureDialog( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("KPhotoAlbum Feature Status"), Close, Close, parent, name )
{
    QWidget* top = plainPage();
    QHBoxLayout* layout = new QHBoxLayout( top, 10 );

    HelpBrowser* edit = new HelpBrowser( top );
    layout->addWidget( edit );

    QString text = i18n("<h1>Overview</h1>"
                        "<p>Below you may see the list of compile- and runtime features KPhotoAlbum has, and their status:</p>"
                        "%1" ).arg( featureString() );
    text += i18n( "<h1>What can I do if I miss a feature?</h1>"

                  "<p>If you compiled KPhotoAlbum yourself, then please review the sections below to learn what to install "
                  "to get the feature in question. If on the other hand you installed KPhotoAlbum from a binary package, please tell "
                  "whoever made the package about this defect, eventually including the information from the section below.<p>"

                  "<p>In case you are missing a feature and you did not compile KPhotoAlbum yourself, please do consider doing so. "
                  "It really isn't that hard. If you need help compiling KPhotoAlbum, feel free to ask on the "
                  "<a href=\"http://mail.kdab.net/mailman/listinfo/kphotoalbum\">KPhotoAlbum mailing list</a></p>"

                  "<p>The steps to compile KPhotoAlbum can be seen on <a href=\"http://www.kphotoalbum.org/download-source.htm\">"
                  "the KPhotoAlbum home page</a>. If you have never compiled a KDE application, then please ensure that "
                  "you have the developer packages installed, in most distributions they go under names like kdelibs<i>-devel</i></p>" );



    text += i18n( "<h1><a name=\"kipi\">Plug-ins Support</a></h1>"
                 "<p>KPhotoAlbum has a plug-in system with lots of extensions. You may among other things find plug-ins for:"
                  "<ul>"
                  "<li>Writing images to cds or dvd's"
                  "<li>Adjusting timestamps on your images"
                  "<li>Making a calendar featuring your images"
                  "<li>Uploading your images to flickr"
                  "</ul></p>"

                  "<p>The plug-in library is called KIPI, and may be downloaded from the "
                  "<a href=\"http://extragear.kde.org/apps/kipi/\">KIPI Home page</a></p>" );

    text += i18n( "<h1><a name=\"exiv2\">EXIF support</a></h1>"
                  "<p>Images store information like the date the image was shot, the shooting angle, focal length, and shutter-speed "
                  "in what is known as EXIF information.</p>"

                  "<p>KPhotoAlbum uses the <a href=\"http://freshmeat.net/projects/exiv2/\">EXIV2 library</a> "
                  "to read EXIF information from images</p>" );

    text += i18n( "<h1><a name=\"kdcraw\">RAW file support</a></h1>"
                  "<p>In addition to regular JPEG files, KPhotoAlbum supports working with various RAW data formats through the "
                  "<a href=\"http://www.kipi-plugins.org/\">libkdcraw</a> library.</p>");


    text += i18n( "<h1><a name=\"database\">SQL Database Support</a></h1>"
                  "<p>KPhotoAlbum allows you to search using a certain number of EXIF tags. For this KPhotoAlbum "
                  "needs a Sqlite database. Unfortunately, for this to work, you need to run Sqlite version 2.8.16, "
                  "so please make sure the right version is installed on your system."
                  "In addition the qt package for sqlite (e.g.qt-sql-sqlite) must be installed.</p>");

    text += i18n("<h1><a name=\"video\">Video Support</a></h1>"
                 "<p>KPhotoAlbum relies on the KDE plug-in subsystem for support for displaying videos. If this feature is not enabled for you, "
                 "have a look at the "
                 "<a href=\"http://wiki.kde.org/tiki-index.php?page=KPhotoAlbum+Video+Support\">KPhotoAlbum wiki article on video support</a>.</p>");

    text += i18n("<h1><a name=\"thumbnails\">Video Thumbnails Support</a></h1>"
                 "<p>KPhotoAlbum asks the KDE plug-in system for help when it needs to generate a thumbnail for videos. "
                 "If this test fails, then you need to go hunting for packages for your system that contains the name <tt>mplayer</tt> "
                 "or <tt>xine</tt>. Some systems provides the support in a package called <b>libarts1-xine</b></p>"
                 "<p>For even better thumbnail support, please try out "
                 "<a href=\"http://www.kde-apps.org/content/show.php?content=41180\">MPlayerThumbs</a>.</p>");
    edit->setText( text );

    resize( 800, 600 );
}

HelpBrowser::HelpBrowser( QWidget* parent, const char* name )
    :QTextBrowser( parent, name )
{
}

void HelpBrowser::setSource( const QString& name )
{
    if ( name.startsWith( QString::fromLatin1( "#" ) ) )
        QTextBrowser::setSource( name );
    else
        kapp->invokeBrowser( name );
}

bool MainWindow::FeatureDialog::hasKIPISupport()
{
#ifdef HASKIPI
    return true;
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasSQLDBSupport()
{
#ifdef QT_NO_SQL
    return false;
#else
    return true;
#endif

}

bool MainWindow::FeatureDialog::hasEXIV2Support()
{
#ifdef HASEXIV2
    return true;
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasEXIV2DBSupport()
{
#ifdef HASEXIV2
    return Exif::Database::isAvailable();
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasRAWSupport() {
#ifdef HASKDCRAW
    return true;
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasAllFeaturesAvailable()
{
    // Only answer those that are compile time tests, otherwise we will pay a penalty each time we start up.
    return hasKIPISupport() && hasSQLDBSupport() && hasEXIV2Support() && hasEXIV2DBSupport() && hasRAWSupport();
}

struct Data
{
    Data() {}
    Data( const QString& title, const QString tag, bool featureFound )
        : title( title ), tag( tag ), featureFound( featureFound ) {}
    QString title;
    QString tag;
    bool featureFound;
};

QString MainWindow::FeatureDialog::featureString()
{
    QValueList<Data> features;
    features << Data( i18n("Plug-ins available"), QString::fromLatin1("#kipi"),  hasKIPISupport() );
    features << Data( i18n("EXIF info supported"), QString::fromLatin1("#exiv2"), hasEXIV2Support() );
    features << Data( i18n("RAW file decoding"), QString::fromLatin1("#kdcraw"), hasRAWSupport() );
    features << Data( i18n("SQL Database Support"), QString::fromLatin1("#database"), hasSQLDBSupport() );
    features << Data( i18n( "Sqlite Database Support (used for EXIF searches)" ), QString::fromLatin1("#database"),
                      hasEXIV2Support() && hasEXIV2DBSupport() );
    features << Data( i18n( "MPEG video support" ), QString::fromLatin1("#video"),  hasVideoSupport( QString::fromLatin1("video/mpeg") ) );
    features << Data( i18n( "Quicktime video support (aka mov)" ), QString::fromLatin1("#video"), hasVideoSupport( QString::fromLatin1("video/quicktime") ) );
    features << Data( i18n( "AVI video support" ), QString::fromLatin1("#video"), hasVideoSupport( QString::fromLatin1("video/x-msvideo") ) );
    features << Data( i18n( "ASF video support (aka wmv)" ), QString::fromLatin1("#video"), hasVideoSupport( QString::fromLatin1("video/x-ms-asf") ) );
    features << Data( i18n( "Real Media"), QString::fromLatin1( "#video" ),
                      hasVideoSupport( QString::fromLatin1( "application/vnd.rn-realmedia" ) )||
                      hasVideoSupport( QString::fromLatin1( "video/vnd.rn-realvideo" ) ) );
    features << Data( i18n( "Video Thumbnails support" ), QString::fromLatin1("#thumbnails"),
                      ImageManager::VideoManager::instance().hasVideoThumbnailSupport() );

    QString result = QString::fromLatin1("<p><table>");
    const QString yes = i18n("Yes");
    const QString no =  QString::fromLatin1("<font color=\"red\">%1</font>").arg( i18n("No") );
    for( QValueList<Data>::ConstIterator featureIt = features.begin(); featureIt != features.end(); ++featureIt ) {
        result += QString::fromLatin1( "<tr><td><a href=\"%1\">%2</a></td><td><b>%3</b></td></tr>" )
                  .arg( (*featureIt).tag ).arg( (*featureIt).title ).arg( (*featureIt).featureFound ? yes : no  );
    }
    result += QString::fromLatin1( "</table></p>" );

    return result;
}

bool MainWindow::FeatureDialog::hasVideoSupport( const QString& mimeType )
{
    static QMap<QString, bool> cache;
    if ( cache.contains( mimeType ) )
        return cache[mimeType];

    KService::Ptr service = KServiceTypeProfile::preferredService( mimeType, QString::fromLatin1("KParts/ReadOnlyPart"));
    if ( !service.data() ) {
        cache[mimeType] = false;
        return false;
    }

    QString library=service->library();
    if ( library.isNull() ) {
        cache[mimeType] = false;
        return false;
    }

    KParts::ReadOnlyPart* part = KParts::ComponentFactory::createPartInstanceFromService<KParts::ReadOnlyPart>(service);
    delete part;
    if ( !part ) {
        cache[mimeType] = false;
        return false;
    }

    cache[mimeType] = true;
    return true;
}

#include "FeatureDialog.moc"

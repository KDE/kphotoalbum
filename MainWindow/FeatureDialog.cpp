/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "FeatureDialog.h"
#include <QDebug>
#include <config-kpa-kipi.h>
#include <config-kpa-exiv2.h>
#include <klocale.h>
#include <qlayout.h>
#include <QList>
#include <kapplication.h>
#include "Exif/Database.h"
#include <ImageManager/VideoManager.h>
#include <kparts/componentfactory.h>
#include <ktoolinvocation.h>
#include <phonon/backendcapabilities.h>

using namespace MainWindow;

FeatureDialog::FeatureDialog( QWidget* parent )
    :KDialog( parent )
{
    setWindowTitle( makeStandardCaption( i18n("Feature Status"), this ) );

    HelpBrowser* edit = new HelpBrowser( this );
    setMainWidget( edit );

    QString text = i18n("<h1>Overview</h1>"
                        "<p>Below you may see the list of compile- and runtime features KPhotoAlbum has, and their status:</p>"
                        "%1", featureString() );
    text += i18n( "<h1>What can I do if I miss a feature?</h1>"

                  "<p>If you compiled KPhotoAlbum yourself, then please review the sections below to learn what to install "
                  "to get the feature in question. If on the other hand you installed KPhotoAlbum from a binary package, please tell "
                  "whoever made the package about this defect, eventually including the information from the section below.<p>"

                  "<p>In case you are missing a feature and you did not compile KPhotoAlbum yourself, please do consider doing so. "
                  "It really isn't that hard. If you need help compiling KPhotoAlbum, feel free to ask on the "
                  "<a href=\"http://mail.kdab.net/mailman/listinfo/kphotoalbum\">KPhotoAlbum mailing list</a></p>"

                  "<p>The steps to compile KPhotoAlbum can be seen on <a href=\"http://www.kphotoalbum.org/index.php?page=compile\">"
                  "the KPhotoAlbum home page</a>. If you have never compiled a KDE application, then please ensure that "
                  "you have the developer packages installed, in most distributions they go under names like kdelibs<i>-devel</i></p>" );



    text += i18n( "<h1><a name=\"kipi\">Plug-ins Support</a></h1>"
                 "<p>KPhotoAlbum has a plug-in system with lots of extensions. You may among other things find plug-ins for:"
                  "<ul>"
                  "<li>Writing images to cds or dvd's"
                  "<li>Adjusting timestamps on your images"
                  "<li>Making a calendar featuring your images"
                  "<li>Uploading your images to flickr"
                  "<li>Upload your images to facebook"
                  "</ul></p>"

                  "<p>The plug-in library is called KIPI, and may be downloaded from the "
                  "<a href=\"http://www.kipi-plugins.org\">KIPI Home page</a></p>" );

    text += i18n( "<h1><a name=\"exiv2\">EXIF support</a></h1>"
                  "<p>Images store information like the date the image was shot, the shooting angle, focal length, and shutter-speed "
                  "in what is known as EXIF information.</p>"

                  "<p>KPhotoAlbum uses the <a href=\"http://freshmeat.net/projects/exiv2/\">EXIV2 library</a> "
                  "to read EXIF information from images</p>" );


    text += i18n( "<h1><a name=\"database\">SQL Database Support</a></h1>"
                  "<p>KPhotoAlbum allows you to search using a certain number of EXIF tags. For this KPhotoAlbum "
                  "needs an Sqlite database. "
                  "In addition the qt package for sqlite (e.g.qt-sql-sqlite) must be installed.</p>");

    text += i18n("<h1><a name=\"thumbnails\">Video Thumbnails Support</a></h1>"
                 "<p>KPhotoAlbum asks the KDE plug-in system for help when it needs to generate a thumbnail for videos. "
                 "<p>Unfortunately KDE4 does currently not come with any plug-ins for this, you therefore need manually to install "
                 "<a href=\"http://www.kde-apps.org/content/show.php?content=41180\">MPlayerThumbs</a>.</p>");

    text += i18n("<h1><a name=\"video\">Video Support</a></h1>"
                 "<p>KPhotoAlbum relies on Qt's Phonon architecture for displaying videos; this in turn relies on GStreamer. "
                 "If this feature is not enabled for you, have a look at the "
                 "<a href=\"http://userbase.kde.org/KPhotoAlbum#Video_Support\">KPhotoAlbum wiki article on video support</a>.</p>");

    QStringList mimeTypes = supportedVideoMimeTypes();
    mimeTypes.sort();
    if ( mimeTypes.isEmpty() )
        text += i18n( "<p>No video mime types found, which indicates that either Qt was compiled without phonon support, or there were missing codecs</p>");
    else
        text += i18n("<p>Phonon is capable of playing movies of these mime types:<ul><li>%1</ul></p>", mimeTypes.join(QString::fromLatin1( "<li>" ) ) );

    edit->setText( text );

    resize( 800, 600 );
}

HelpBrowser::HelpBrowser( QWidget* parent, const char* name )
    :KTextBrowser( parent )
{
    setObjectName(QString::fromLatin1(name));
}

void HelpBrowser::setSource( const QUrl& url )
{
    const QString name = url.toString();

    if ( name.startsWith( QString::fromLatin1( "#" ) ) )
        KTextBrowser::setSource( name );
    else
        KToolInvocation::invokeBrowser( name );
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
#ifdef HAVE_EXIV2
    return true;
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasEXIV2DBSupport()
{
#ifdef HAVE_EXIV2
    return Exif::Database::isAvailable();
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasAllFeaturesAvailable()
{
    // Only answer those that are compile time tests, otherwise we will pay a penalty each time we start up.
    return hasKIPISupport() && hasSQLDBSupport() && hasEXIV2Support() && hasEXIV2DBSupport();
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
     QList<Data> features;
    features << Data( i18n("Plug-ins available"), QString::fromLatin1("#kipi"),  hasKIPISupport() );
    features << Data( i18n("EXIF info supported"), QString::fromLatin1("#exiv2"), hasEXIV2Support() );
    features << Data( i18n("SQL Database Support"), QString::fromLatin1("#database"), hasSQLDBSupport() );
    features << Data( i18n( "Sqlite Database Support (used for EXIF searches)" ), QString::fromLatin1("#database"),
                      hasEXIV2Support() && hasEXIV2DBSupport() );
    features << Data( i18n( "Video Thumbnail support" ), QString::fromLatin1("#thumbnails"),
                      ImageManager::VideoManager::instance().hasVideoThumbnailSupport() );
    features << Data( i18n( "Video support" ), QString::fromLatin1("#video"),  !supportedVideoMimeTypes().isEmpty() );

    QString result = QString::fromLatin1("<p><table>");
    const QString yes = i18n("Yes");
    const QString no =  QString::fromLatin1("<font color=\"red\">%1</font>").arg( i18n("No") );
     for( QList<Data>::ConstIterator featureIt = features.constBegin(); featureIt != features.constEnd(); ++featureIt ) {
        result += QString::fromLatin1( "<tr><td><a href=\"%1\">%2</a></td><td><b>%3</b></td></tr>" )
                  .arg( (*featureIt).tag ).arg( (*featureIt).title ).arg( (*featureIt).featureFound ? yes : no  );
    }
    result += QString::fromLatin1( "</table></p>" );

    return result;
}

QStringList MainWindow::FeatureDialog::supportedVideoMimeTypes()
{
    return Phonon::BackendCapabilities::availableMimeTypes();
}

#include "FeatureDialog.moc"

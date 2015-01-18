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
#include <config-kpa-kface.h>
#include <config-kpa-kgeomap.h>
#include <klocale.h>
#include <qlayout.h>
#include <QList>
#include <kapplication.h>
#include "Exif/Database.h"
#include <kparts/componentfactory.h>
#include <ktoolinvocation.h>
#include <phonon/backendcapabilities.h>
#include <KStandardDirs>
#include <QProcess>

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
                  "whoever made the package about this defect, eventually including the information from the section below.</p>"

                  "<p>In case you are missing a feature and you did not compile KPhotoAlbum yourself, please do consider doing so. "
                  "It really is not that hard. If you need help compiling KPhotoAlbum, feel free to ask on the "
                  "<a href=\"http://mail.kdab.com/mailman/listinfo/kphotoalbum\">KPhotoAlbum mailing list</a></p>"

                  "<p>The steps to compile KPhotoAlbum can be seen on <a href=\"http://www.kphotoalbum.org/index.php?page=compile\">"
                  "the KPhotoAlbum home page</a>. If you have never compiled a KDE application, then please ensure that "
                  "you have the developer packages installed, in most distributions they go under names like kdelibs<i>-devel</i></p>" );



    text += i18n( "<h1><a name=\"kipi\">Plug-ins support</a></h1>"
                 "<p>KPhotoAlbum has a plug-in system with lots of extensions. You may among other things find plug-ins for:"
                  "<ul>"
                  "<li>Writing images to cds or dvd's</li>"
                  "<li>Adjusting timestamps on your images</li>"
                  "<li>Making a calendar featuring your images</li>"
                  "<li>Uploading your images to flickr</li>"
                  "<li>Upload your images to facebook</li>"
                  "</ul></p>"

                  "<p>The plug-in library is called KIPI, and may be downloaded from the "
                  "<a href=\"http://userbase.kde.org/KIPI\">KDE Userbase Wiki</a></p>" );

    text += i18n( "<h1><a name=\"exiv2\">EXIF support</a></h1>"
                  "<p>Images store information like the date the image was shot, the shooting angle, focal length, and shutter-speed "
                  "in what is known as EXIF information.</p>"

                  "<p>KPhotoAlbum uses the <a href=\"http://www.exiv2.org/\">EXIV2 library</a> "
                  "to read EXIF information from images</p>" );


    text += i18n( "<h1><a name=\"database\">SQLite database support</a></h1>"
                  "<p>KPhotoAlbum allows you to search using a certain number of EXIF tags. For this KPhotoAlbum "
                  "needs an Sqlite database. "
                  "In addition the qt package for sqlite (e.g.qt-sql-sqlite) must be installed.</p>");

    text += i18n("<h1><a name=\"kface\">Face detection and recognition support</a></h1>"
                 "<p>If KPhotoAlbum has been built with support for libkface, "
                 "face detection and recognition features are enabled in the annotation dialog."
                 "</p>");

    text += i18n("<h1><a name=\"geomap\">Map view for geotagged images</a></h1>"
                 "<p>If KPhotoAlbum has been built with support for libkgeomap, "
                 "KPhotoAlbum can show images with GPS information on a map."
                 "</p>");

    text += i18n("<h1><a name=\"video\">Video support</a></h1>"
                 "<p>KPhotoAlbum relies on Qt's Phonon architecture for displaying videos; this in turn relies on GStreamer. "
                 "If this feature is not enabled for you, have a look at the "
                 "<a href=\"http://userbase.kde.org/KPhotoAlbum#Video_Support\">KPhotoAlbum wiki article on video support</a>.</p>");

    QStringList mimeTypes = supportedVideoMimeTypes();
    mimeTypes.sort();
    if ( mimeTypes.isEmpty() )
        text += i18n( "<p>No video mime types found, which indicates that either Qt was compiled without phonon support, or there were missing codecs</p>");
    else
        text += i18n("<p>Phonon is capable of playing movies of these mime types:<ul><li>%1</li></ul></p>", mimeTypes.join(QString::fromLatin1( "</li><li>" ) ) );

    text += i18n("<h1><a name=\"videoPreview\">Video thumbnail support</a></h1>"
                 "<p>KPhotoAlbum uses <tt>MPlayer</tt> to extract thumbnails from videos. These thumbnails are used to preview "
                 "videos in the thumbnail viewer.</p>"
                 "<p>If at all possible you should install the <b>MPlayer2</b> package rather than the <b>MPlayer</b> package, as it has important "
                 "improvements over the MPlayer package. MPlayer (in contrast to MPlayer2) often has problems extracting the length "
                 "of videos and also often fails to extract the thumbnails used for cycling video thumbnails.</p>");

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

    if ( name.startsWith( QString::fromLatin1( "#" ) ) ) {
        // Must be QTextBrowser rather than KTextBrowser, as KTextBrowser opens the URL in an external browser, rather than jumping to the target.
        QTextBrowser::setSource( name ); //krazy:exclude=qclasses
    }
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

bool MainWindow::FeatureDialog::hasKfaceSupport()
{
#ifdef HAVE_KFACE
    return true;
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasGeoMapSupport()
{
#ifdef HAVE_KGEOMAP
    return true;
#else
    return false;
#endif
}

QString FeatureDialog::mplayerBinary()
{
    const QString mplayer2 = KStandardDirs::findExe(QString::fromLatin1("mplayer2"));

    if ( !mplayer2.isNull() )
        return mplayer2;
    else
        return KStandardDirs::findExe(QString::fromLatin1("mplayer"));
}

bool FeatureDialog::isMplayer2()
{
    QProcess process;
    process.start( mplayerBinary(), QStringList() << QString::fromLatin1("--version"));
    process.waitForFinished();
    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput().data());
    return output.contains(QString::fromLatin1("MPlayer2"));
}

bool MainWindow::FeatureDialog::hasAllFeaturesAvailable()
{
    // Only answer those that are compile time tests, otherwise we will pay a penalty each time we start up.
    return hasKIPISupport() && hasEXIV2Support() && hasEXIV2DBSupport() && hasKfaceSupport() && hasGeoMapSupport() && !mplayerBinary().isNull() && isMplayer2();
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
    features << Data( i18n( "Sqlite database support (used for EXIF searches)" ), QString::fromLatin1("#database"), hasEXIV2DBSupport() );
    features << Data( i18n( "Face detection and recognition support" ), QString::fromLatin1("#kface"),  hasKfaceSupport() );
    features << Data( i18n( "Map view for geotagged images." ), QString::fromLatin1("#geomap"),  hasGeoMapSupport() );
    features << Data( i18n( "Video support" ), QString::fromLatin1("#video"),  !supportedVideoMimeTypes().isEmpty() );

    QString result = QString::fromLatin1("<p><table>");
    const QString red = QString::fromLatin1("<font color=\"red\">%1</font>");
    const QString yes = i18nc("Feature available","Yes");
    const QString no =  red.arg( i18nc("Feature not available","No") );
    const QString formatString = QString::fromLatin1( "<tr><td><a href=\"%1\">%2</a></td><td><b>%3</b></td></tr>" );
     for( QList<Data>::ConstIterator featureIt = features.constBegin(); featureIt != features.constEnd(); ++featureIt ) {
        result += formatString
                  .arg( (*featureIt).tag ).arg( (*featureIt).title ).arg( (*featureIt).featureFound ? yes : no  );
    }

     QString thumbnailSupport = mplayerBinary().isNull() ? no : ( isMplayer2() ? yes : red.arg(i18n("Only with MPlayer1")));
    result += formatString.arg(QString::fromLatin1("#videoPreview")).arg(i18n("Video thumbnail support")).arg(thumbnailSupport);
    result += QString::fromLatin1( "</table></p>" );

    return result;
}

QStringList MainWindow::FeatureDialog::supportedVideoMimeTypes()
{
    return Phonon::BackendCapabilities::availableMimeTypes();
}

#include "FeatureDialog.moc"
// vi:expandtab:tabstop=4 shiftwidth=4:

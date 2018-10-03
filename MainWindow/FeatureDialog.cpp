/* Copyright (C) 2003-2018 Jesper K. Pedersen <blackie@kde.org>

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

#include <config-kpa-kipi.h>
#include <config-kpa-kgeomap.h>
#include "FeatureDialog.h"

#include <QDialogButtonBox>
#include <QLayout>
#include <QList>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <phonon/backendcapabilities.h>

#include "Exif/Database.h"

using namespace MainWindow;

FeatureDialog::FeatureDialog( QWidget* parent )
    :QDialog( parent )
{
    setWindowTitle( i18nc("@title:window", "Feature Status") );

    QTextBrowser* browser = new QTextBrowser( this );

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

    text += i18n( "<h1><a name=\"database\">SQLite database support</a></h1>"
                  "<p>KPhotoAlbum allows you to search using a certain number of Exif tags. For this KPhotoAlbum "
                  "needs an SQLite database. "
                  "In addition the Qt package for SQLite (e.g. qt-sql-sqlite) must be installed.</p>");

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
                 "<p>KPhotoAlbum can use <tt>ffmpeg</tt> or <tt>MPlayer</tt> to extract thumbnails from videos. These thumbnails are used to preview "
                 "videos in the thumbnail viewer.</p>"
                 "<p>In the past, MPlayer (in contrast to MPlayer2) often had problems extracting the length "
                 "of videos and also often fails to extract the thumbnails used for cycling video thumbnails. "
                 "For that reason, you should prefer ffmpeg or MPlayer2 over MPlayer, if possible.</p>"
                 );

    text += i18n("<h1><a name=\"videoInfo\">Video metadata support</a></h1>"
                 "<p>KPhotoAlbum can use <tt>ffprobe</tt> or <tt>MPlayer</tt> to extract length information from videos."
                 "</p>"
                 "<p>Correct length information is also necessary for correct rendering of video thumbnails.</p>"
                 );

    browser->setText( text );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(browser);
    this->setLayout(layout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setShortcut(Qt::CTRL | Qt::Key_Return);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

QSize FeatureDialog::sizeHint() const
{
    return QSize(800,600);
}

bool MainWindow::FeatureDialog::hasKIPISupport()
{
#ifdef HASKIPI
    return true;
#else
    return false;
#endif
}

bool MainWindow::FeatureDialog::hasEXIV2DBSupport()
{
    return Exif::Database::isAvailable();
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
    QString mplayer = QStandardPaths::findExecutable( QString::fromLatin1("mplayer2"));

    if ( mplayer.isNull() )
        mplayer = QStandardPaths::findExecutable( QString::fromLatin1("mplayer"));

    return mplayer;
}

bool FeatureDialog::isMplayer2()
{
    QProcess process;
    process.start( mplayerBinary(), QStringList() << QString::fromLatin1("--version"));
    process.waitForFinished();
    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput().data());
    return output.contains(QString::fromLatin1("MPlayer2"));
}

QString FeatureDialog::ffmpegBinary()
{
    QString ffmpeg = QStandardPaths::findExecutable( QString::fromLatin1("ffmpeg"));
    return ffmpeg;
}

QString FeatureDialog::ffprobeBinary()
{
    QString ffprobe = QStandardPaths::findExecutable( QString::fromLatin1("ffprobe"));
    return ffprobe;
}

bool FeatureDialog::hasVideoThumbnailer()
{
    return ! ( ffmpegBinary().isEmpty() && mplayerBinary().isEmpty());
}

bool FeatureDialog::hasVideoProber()
{
    return ! ( ffprobeBinary().isEmpty() && mplayerBinary().isEmpty());
}

bool MainWindow::FeatureDialog::hasAllFeaturesAvailable()
{
    // Only answer those that are compile time tests, otherwise we will pay a penalty each time we start up.
    return hasKIPISupport() && hasEXIV2DBSupport() && hasGeoMapSupport() && hasVideoThumbnailer() && hasVideoProber();
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
    features << Data( i18n( "SQLite database support (used for Exif searches)" ), QString::fromLatin1("#database"), hasEXIV2DBSupport() );
    features << Data( i18n( "Map view for geotagged images." ), QString::fromLatin1("#geomap"),  hasGeoMapSupport() );
    features << Data( i18n( "Video support" ), QString::fromLatin1("#video"),  !supportedVideoMimeTypes().isEmpty() );

    QString result = QString::fromLatin1("<p><table>");
    const QString red = QString::fromLatin1("<font color=\"red\">%1</font>");
    const QString yellow = QString::fromLatin1("<font color=\"yellow\">%1</font>");
    const QString yes = i18nc("Feature available","Yes");
    const QString no =  red.arg( i18nc("Feature not available","No") );
    const QString formatString = QString::fromLatin1( "<tr><td><a href=\"%1\">%2</a></td><td><b>%3</b></td></tr>" );
     for( QList<Data>::ConstIterator featureIt = features.constBegin(); featureIt != features.constEnd(); ++featureIt ) {
        result += formatString
                  .arg( (*featureIt).tag ).arg( (*featureIt).title ).arg( (*featureIt).featureFound ? yes : no  );
    }

     QString thumbnailSupport = hasVideoThumbnailer() ? ( !ffmpegBinary().isEmpty() || isMplayer2() ? yes : yellow.arg(i18n("Only with MPlayer1"))) : no ;
    result += formatString.arg(QString::fromLatin1("#videoPreview")).arg(i18n("Video thumbnail support")).arg(thumbnailSupport);

     QString videoinfoSupport = hasVideoProber() ? yes : no;
    result += formatString.arg(QString::fromLatin1("#videoInfo")).arg(i18n("Video metadata support")).arg(videoinfoSupport);
    result += QString::fromLatin1( "</table></p>" );

    return result;
}

QStringList MainWindow::FeatureDialog::supportedVideoMimeTypes()
{
    return Phonon::BackendCapabilities::availableMimeTypes();
}

// vi:expandtab:tabstop=4 shiftwidth=4:

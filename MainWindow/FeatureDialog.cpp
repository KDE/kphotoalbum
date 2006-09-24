#include "FeatureDialog.h"
#include <config.h>
#include <klocale.h>
#include <qlayout.h>
#include <kapplication.h>
#include "Exif/Database.h"
#include <kuserprofile.h>

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
                  "whoever made the package about this defeact, eventually including the information from the section below.<p>"

                  "<p>In case you are missing a feature and you did not compile KPhotoAlbum yourself, please do consider doing so. "
                  "It really isn't that hard. If you need help compiling KPhotoAlbum, feel free to ask on the "
                  "<a href=\"http://mail.kdab.net/mailman/listinfo/kphotoalbum\">KPhotoAlbum mailing list</a></p>"

                  "<p>The steps to compile KPhotoAlbum can be seen on <a href=\"http://www.kphotoalbum.org/download-source.htm\">"
                  "the KPhotoAlbum home page</a>. If you have never compiled a KDE application, then please ensure that "
                  "you have the developer packages installed, in most distributions they go under names like kdelibs<i>-devel</i></p>" );



    text += i18n( "<h1><a name=\"kipi\">Plug-ins Support</a></h1>"
                 "<p>KPhotoAlbum has a plug-in system with lots of extensions. You may among other things find plug-ins for:"
                  "<ul>"
                  "<li>Writting images to cds or dvd's"
                  "<li>Adjusting timestamps on your images"
                  "<li>Making a calender with your images in it"
                  "<li>Uploading your images to flickr"
                  "</ul></p>"

                  "<p>The plug-in library is called KIPI, and may be downloaded from the "
                  "<a href=\"http://extragear.kde.org/apps/kipi/\">KIPI Home page</a></p>" );

    text += i18n( "<h1><a name=\"exiv2\">EXIF support</a></h1>"
                  "<p>Images store information like date image was shot, angle it was shot with, focal length, and shutter time "
                  "in what is know as EXIF information.</p>"

                  "<p>If you do not care about focal length, ISO speed, etc, then you might well live without these features in KPhotoAlbum, "
                  "be, however, aware that KPhotoAlbum will then use the time stamp of the image files to identify when the image was taken. "
                  "This time stamp might be wrong in case you moved the image arround for example.</p>"

                  "<p>KPhotoAlbum uses the <a href=\"http://freshmeat.net/projects/exiv2/\">EXIV2 library</a> "
                  "for reading EXIF information from images</p>"
        );


    text += i18n( "<h1><a name=\"database\">SQL Database Support</a></h1>"
                  "KPhotoAlbum allows you to search using a certain number of EXIF tags. For this KPhotoAlbum "
                  "needs a Sqlite database. Unfortunately, for this to work, you need to run Sqlite version 2.8.16, "
                  "so please make sure this is the right version installed on your system." );
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

bool MainWindow::FeatureDialog::hasAllFeaturesAvailable()
{
    return hasKIPISupport() && hasSQLDBSupport() && hasEXIV2Support() && hasEXIV2DBSupport();
}

QString MainWindow::FeatureDialog::featureString()
{
    QString yes = QString::fromLatin1( "<b>%1</b>" ).arg(i18n("Yes"));
    QString no = QString::fromLatin1( "<b><font color=\"red\">%1</font></b>" ).arg( i18n("No") );

    QString hasKipi = hasKIPISupport() ? yes : no;
    QString hasDatabaseSupport = hasSQLDBSupport() ? yes : no;
    QString hasEXIV2 = hasEXIV2Support() ? yes : no;
    QString hasExifDatabase = QString::fromLatin1( "<b><font color=\"red\">%1</b></font>" )
                              .arg( i18n("untested - missing EXIF support") );
    if ( hasEXIV2Support() )
        hasExifDatabase = hasEXIV2DBSupport() ? yes : no;
    QString mpegSupport = hasVideoSupport( QString::fromLatin1("video/mpeg") ) ? yes : no;
    QString rpSupport = hasVideoSupport( QString::fromLatin1("video/real") ) ? yes : no;


    QString result = QString::fromLatin1( "<p><table>"
                           "<tr><td><a href=\"#kipi\">%1</a></td><td>%1</tr></tr>"
                           "<tr><td><a href=\"#exiv2\">%1</a></td><td>%1</td></tr>"
                           "<tr><td><a href=\"#database\">%1</a></td><td>%1</td></tr>"
                           "<tr><td><a href=\"#database\">%1</a></td><td>%1</td></tr>"
                           "<tr><td><a href=\"#mpeg\">%1</a></td><td>%1</td></tr>"
                           "<tr><td><a href=\"#rp\">%1</a></td><td>%1</td></tr>"
                           "</table></p>")
                     .arg( i18n("Plug-ins available") ).arg( hasKipi )
                     .arg( i18n("EXIF info supported") ).arg( hasEXIV2 )
                     .arg( i18n("SQL Database Support") ).arg( hasDatabaseSupport )
                     .arg( i18n( "Sqlite Database Support (used for EXIF searches)" ) ).arg( hasExifDatabase )
                     .arg( i18n( "Inline MPEG video support" ) ).arg( mpegSupport )
                     .arg( i18n( "Inline Real Player video support" ) ).arg( rpSupport );

    return result;
}

bool MainWindow::FeatureDialog::hasVideoSupport( const QString& mimeType )
{
    KService::Ptr service = KServiceTypeProfile::preferredService( mimeType, QString::fromLatin1("KParts/ReadOnlyPart"));
    return service.data();
}

#include "FeatureDialog.moc"

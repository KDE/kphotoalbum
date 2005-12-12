#include "featuredialog.h"
#include <klocale.h>
#include <qlayout.h>
#include <qtextbrowser.h>
#include <kapplication.h>
#include <qfeatures.h>
#include "Exif/Database.h"
#include <config.h>

FeatureDialog::FeatureDialog( QWidget* parent, const char* name )
    :KDialogBase( Plain, i18n("KimDaBa Feature Status"), Close, Close, parent, name )
{
    QWidget* top = plainPage();
    QHBoxLayout* layout = new QHBoxLayout( top, 10 );

    HelpBrowser* edit = new HelpBrowser( top );
    layout->addWidget( edit );

    QString yes = QString::fromLatin1( "<b>%1</b>" ).arg(i18n("Yes"));
    QString no = QString::fromLatin1( "<b><font color=\"red\">%1</font></b>" ).arg( i18n("No") );

    QString hasKipi = no;
#ifdef HASKIPI
    hasKipi = yes;
#endif

    QString hasEXIV2 = no;
    QString hasExifDatabase = QString::fromLatin1( "<b><font color=\"red\">%1</b></font>" ).arg( i18n("untested - missing EXIF support") );

#ifdef HASEXIV2
    hasEXIV2 = yes;
    hasExifDatabase = Exif::Database::isAvailable() ? yes : no;
#endif

    QString hasDatabaseSupport = yes;
#ifdef QT_NO_SQL
    hasDatabaseSupport = no;
#endif


    QString text =
        i18n("<h1>Overview</h1>"
             "<p>Below you may see the list of compile- and runtime features KimDaBa has, and their status:</p>"

             "<p><table>"
             "<tr><td><a href=\"#kipi\">Plug-ins available</a></td><td>%1</tr></tr>"
             "<tr><td><a href=\"#exiv2\">EXIF info supported</a></td><td>%2</td></tr>"
             "<tr><td><a href=\"#database\">SQL Database Support</a></td><td>%3</td></tr>"
             "<tr><td><a href=\"#database\">Sqlite Database Support</a></td><td>%4</td></tr>"
             "</table></p>" )
        .arg( hasKipi )
        .arg( hasEXIV2 )
        .arg( hasDatabaseSupport )
        .arg( hasExifDatabase );


    text += i18n( "<h1>What can I do if I miss a feature?</h1>"

                  "<p>If you compiled KimDaBa yourself, then please review the sections below to learn what to install "
                  "to get the feature in question. If on the other hand you installed KimDaBa from a binary package, please tell "
                  "whoever made the package about this defeact, eventually including the information from the section below.<p>"

                  "<p>In case you are missing a feature and you did not compile KimDaBa yourself, please do consider doing so. "
                  "It really isn't that hard. If you need help compiling KimDaBa, feel free to ask on the "
                  "<a href=\"http://mail.kdab.net/mailman/listinfo/kimdaba\">KimDaBa mailing list</a></p>"

                  "<p>The steps to compile KimDaBa can be seen on <a href=\"http://ktown.kde.org/kimdaba/download-source.htm\">"
                  "the KimDaBa home page</a>. If you have never compiled a KDE application, then please ensure that "
                  "you have the developer packages installed, in most distributions they go under names like kdelibs<i>-devel</i></p>" );



    text += i18n( "<h1><a name=\"kipi\">Plug-ins Support</a></h1>"
                 "<p>KimDaBa has a plug-in system with lots of extensions. You may among other things find plug-ins for:"
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

                  "<p>If you do not care about focal length, ISO speed, etc, then you might well live without these features in KimDaBa, "
                  "be, however, aware that KimDaBa will then use the time stamp of the image files to identify when the image was taken. "
                  "This time stamp might be wrong in case you moved the image arround for example.</p>"

                  "<p>KimDaBa uses the <a href=\"http://freshmeat.net/projects/exiv2/\">EXIV2 library</a> "
                  "for reading EXIF information from images</p>"
        );


    text += i18n( "<h1><a name=\"database\">SQL Database Support</a></h1>"
                  "KimDaBa allows you to search using a certain number of EXIF tags. For this KimDaBa "
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

#include "featuredialog.moc"

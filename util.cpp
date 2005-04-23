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
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "util.h"
#include "options.h"
#include "imageinfo.h"
#include <klocale.h>
#include <qfileinfo.h>
#include <kmessagebox.h>
#include <qurl.h>
#include <kapplication.h>
#include <unistd.h>
#include <qmutex.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <stdlib.h>
#include <qregexp.h>
#include <kimageio.h>
#include <kcmdlineargs.h>
#include <kio/netaccess.h>
#include "mainview.h"

extern "C" {
#define XMD_H // prevent INT32 clash from jpeglib
#include <jpeglib.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <setjmp.h>
#include <sys/types.h>
#include "categorycollection.h"
}

bool Util::writeOptions( QDomDocument doc, QDomElement elm, QMap<QString, QStringList>& options,
                         CategoryCollection* categories )
{
    bool anyAtAll = false;
    QStringList grps = CategoryCollection::instance()->categoryNames();
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        QString name = *it;
        opt.setAttribute( QString::fromLatin1("name"),  name );

        if ( categories ) {
            opt.setAttribute( QString::fromLatin1( "icon" ), categories->categoryForName(name)->iconName() );
            opt.setAttribute( QString::fromLatin1( "show" ), categories->categoryForName(name)->doShow() );
            opt.setAttribute( QString::fromLatin1( "viewsize" ), categories->categoryForName(name)->viewSize() );
            opt.setAttribute( QString::fromLatin1( "viewtype" ), categories->categoryForName(name)->viewType() );
        }

        // we don t save the values for the option "Folder" since it is automatically set
        // but we keep the <option> element to allow to save user's preference for viewsize,icon,show,name
        QStringList list;
        if ( name== QString::fromLatin1("Folder") )
            list = QStringList();
        else
            list = options[name];
        bool any = false;
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *it2 );
            opt.appendChild( val );
            any = true;
            anyAtAll = true;
        }
        if ( any || categories  ) // We always want to write all records when writing from Options
            elm.appendChild( opt );
    }
    return anyAtAll;
}



void Util::readOptions( QDomElement elm, QMap<QString, QStringList>* options,
                        CategoryCollection* categories )
{
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "options" ) );

    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull();
          nodeOption = nodeOption.nextSibling() )  {

        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            Q_ASSERT( elmOption.tagName() == QString::fromLatin1("option") );
            QString name = elmOption.attribute( QString::fromLatin1("name") );
            if ( name == QString::fromLatin1( "Folder" ) )
                continue; // KimDaBa 2.0 save this to the file, that was a mistake.

            if ( !name.isNull() )  {
                // Read Category info
                if ( categories ) {
                    QString icon= elmOption.attribute( QString::fromLatin1("icon") );
                    Category::ViewSize size =
                        (Category::ViewSize) elmOption.attribute( QString::fromLatin1("viewsize"), QString::fromLatin1( "0" ) ).toInt();
                    Category::ViewType type =
                        (Category::ViewType) elmOption.attribute( QString::fromLatin1("viewtype"), QString::fromLatin1( "0" ) ).toInt();
                    bool show = (bool) elmOption.attribute( QString::fromLatin1( "show" ),
                                                            QString::fromLatin1( "1" ) ).toInt();

                    Q_ASSERT( !categories->categoryForName( name ) );
                    Category* cat = new Category( name, icon, size, type, show );
                    categories->addCategory( cat );
                }

                // Read values
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull();
                      nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == QString::fromLatin1("value") );
                        QString value = elmValue.attribute( QString::fromLatin1("value") );
                        if ( !value.isNull() )  {
                            (*options)[name].append( value );
                        }
                    }
                }
            }
        }
    }
}

QString Util::createInfoText( ImageInfo* info, QMap< int,QPair<QString,QString> >* linkMap )
{
    Q_ASSERT( info );
    QString text;
    if ( Options::instance()->showDate() )  {
        if ( info->startDate().isNull() ) {
            // Don't append anything
        }
        else if ( info->endDate().isNull() )
            text += info->startDate();
        else
            text += i18n("date1 to date2", "%1 to %2").arg( info->startDate() ).arg( info->endDate() );

        if ( !text.isEmpty() ) {
            text = i18n("<b>Date: </b> ") + text + QString::fromLatin1("<br>");
        }
    }

    QStringList grps = CategoryCollection::instance()->categoryNames();
    int link = 0;
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QString category = *it;
        if ( Options::instance()->showOption( category ) ) {
            QStringList items = info->optionValue( category );
            if (items.count() != 0 ) {
                text += QString::fromLatin1( "<b>%1: </b> " )
                        .arg( CategoryCollection::instance()->categoryForName( category )->text() );
                bool first = true;
                for( QStringList::Iterator it2 = items.begin(); it2 != items.end(); ++it2 ) {
                    QString item = *it2;
                    if ( first )
                        first = false;
                    else
                        text += QString::fromLatin1( ", " );

                    if ( linkMap ) {
                        ++link;
                        (*linkMap)[link] = QPair<QString,QString>( category, item );
                        text += QString::fromLatin1( "<a href=\"%1\">%2</a>")
                                .arg( link ).arg( item );
                    }
                    else
                        text += item;
                }
                text += QString::fromLatin1( "<br>" );
            }
        }
    }

    if ( Options::instance()->showDescription() && !info->description().isEmpty())  {
        if ( !text.isEmpty() )
            text += i18n("<b>Description: </b> ") +  info->description() + QString::fromLatin1("<br>");
    }

    return text;
}

void Util::checkForBackupFile( const QString& fileName )
{
    QString backupName = QFileInfo( fileName ).dirPath( true ) + QString::fromLatin1("/.#") + QFileInfo( fileName ).fileName();
    QFileInfo backUpFile( backupName);
    QFileInfo indexFile( fileName );
    if ( !backUpFile.exists() || indexFile.lastModified() > backUpFile.lastModified() )
        return;

    int code = KMessageBox::questionYesNo( 0, i18n("Backup file '%1' exists and is newer than '%2'. "
                                                   "Should I use the backup file?")
                                           .arg(backupName).arg(fileName),
                                           i18n("Found Backup File") );
    if ( code == KMessageBox::Yes ) {
        QFile in( backupName );
        if ( in.open( IO_ReadOnly ) ) {
            QFile out( fileName );
            if (out.open( IO_WriteOnly ) ) {
                char data[1024];
                int len;
                while ( (len = in.readBlock( data, 1024 ) ) )
                    out.writeBlock( data, len );
            }
        }
    }
}

bool Util::ctrlKeyDown()
{
    return KApplication::keyboardModifiers() & KApplication::ControlModifier;
}

QString Util::setupDemo()
{
    QString dir = QString::fromLatin1( "/tmp/kimdaba-demo-" ) + QString::fromLocal8Bit( getenv( "LOGNAME" ) );
    QFileInfo fi(dir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( dir );
        if ( !ok ) {
            KMessageBox::error( 0, i18n("Unable to create directory '%1' needed for demo.").arg( dir ), i18n("Error Running Demo") );
            exit(-1);
        }
    }

    bool ok;

    // index.xml
    QString str = readInstalledFile( QString::fromLatin1( "demo/index.xml" ) );
    if ( str.isNull() )
        exit(-1);

    str = str.replace( QRegExp( QString::fromLatin1("imageDirectory=\"[^\"]*\"")), QString::fromLatin1("imageDirectory=\"%1\"").arg(dir) );
    str = str.replace( QRegExp( QString::fromLatin1("htmlBaseDir=\"[^\"]*\"")), QString::fromLatin1("") );
    str = str.replace( QRegExp( QString::fromLatin1("htmlBaseURL=\"[^\"]*\"")), QString::fromLatin1("") );

    QString configFile = dir + QString::fromLatin1( "/index.xml" );
    if ( ! QFileInfo( configFile ).exists() ) {
        QFile out( configFile );
        if ( !out.open( IO_WriteOnly ) ) {
            KMessageBox::error( 0, i18n("Unable to open '%1' for writing.").arg( configFile ), i18n("Error Running Demo") );
            exit(-1);
        }
        QTextStream( &out ) << str;
        out.close();
    }

    // Images
    QStringList files = KStandardDirs().findAllResources( "data", QString::fromLatin1("kimdaba/demo/*.jpg" ) );
    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
        QString destFile = dir + QString::fromLatin1( "/" ) + QFileInfo(*it).fileName();
        if ( ! QFileInfo( destFile ).exists() ) {
            ok = copy( *it, destFile );
            if ( !ok ) {
                KMessageBox::error( 0, i18n("Unable to copy '%1' to '%2'.").arg( *it ).arg( destFile ), i18n("Error Running Demo") );
                exit(-1);
            }
        }

    }

    // CategoryImages
    dir = dir + QString::fromLatin1("/CategoryImages");
    fi = QFileInfo(dir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( dir  );
        if ( !ok ) {
            KMessageBox::error( 0, i18n("Unable to create directory '%1' needed for demo.").arg( dir ), i18n("Error Running Demo") );
            exit(-1);
        }
    }

    // Category images.
    files = KStandardDirs().findAllResources( "data", QString::fromLatin1("kimdaba/demo/CategoryImages/*.jpg" ) );
    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
        QString destFile = dir + QString::fromLatin1( "/" ) + QFileInfo(*it).fileName();
        if ( ! QFileInfo( destFile ).exists() ) {
            ok = copy( *it, destFile );
            if ( !ok ) {
                KMessageBox::error( 0, i18n("Unable to make symlink from '%1' to '%2'.").arg( *it ).arg( destFile ), i18n("Error Running Demo") );
                exit(-1);
            }
        }

    }

    return configFile;
}

bool Util::copy( const QString& from, const QString& to )
{
    QFile in( from );
    QFile out( to );

    if ( !in.open(IO_ReadOnly) ) {
        return false;
    }
    if ( !out.open(IO_WriteOnly) ) {
        in.close();
        return false;
    }

    char buf[4096];
    while( !in.atEnd() ) {
        unsigned long int len = in.readBlock( buf, sizeof(buf));
        out.writeBlock( buf, len );
    }

    in.close();
    out.close();
    return true;
}

bool Util::makeHardLink( const QString& from, const QString& to )
{
    if (link(from.ascii(), to.ascii()) != 0)
        return false;
    else
        return true;
}

QString Util::readInstalledFile( const QString& fileName )
{
    QString inFileName = locate( "data", QString::fromLatin1( "kimdaba/%1" ).arg( fileName ) );
    if ( inFileName.isEmpty() ) {
        KMessageBox::error( 0, i18n("<qt>Unable to find kimdaba/%1. This is likely an installation error. Did you remember to do a 'make install'? Did you set KDEDIRS, in case you did not install it in the default location?</qt>").arg( fileName ) ); // Proof reader comment: What if it was a binary installation? (eg. apt-get)
        return QString::null;
    }

    QFile file( inFileName );
    if ( !file.open( IO_ReadOnly ) ) {
        KMessageBox::error( 0, i18n("Could not open file %1.").arg( inFileName ) );
        return QString::null;
    }

    QTextStream stream( &file );
    QString content = stream.read();
    file.close();

    return content;
}

QString Util::getThumbnailDir( const QString& imageFile ) {
    return QFileInfo( imageFile ).dirPath() + QString::fromLatin1("/ThumbNails");
}

QString Util::getThumbnailFile( const QString& imageFile, int width, int height, int angle ) {
    QFileInfo info( imageFile );
    while (angle < 0)
        angle += 360;
    angle %= 360;
    return info.dirPath() + QString::fromLatin1("/ThumbNails")+
        QString::fromLatin1("/%1x%2-%3-%4")
        .arg(width)
        .arg(height)
        .arg(angle)
        .arg( info.fileName() );
}

void Util::removeThumbNail( const QString& imageFile )
{
    QFileInfo fi( imageFile );
    QString path = fi.dirPath(true);

    QDir dir( QString::fromLatin1( "%1/ThumbNails" ).arg( path ) );
    QStringList matches = dir.entryList( QString::fromLatin1( "*-%1" ).arg( fi.fileName() ) );
    for( QStringList::Iterator it = matches.begin(); it != matches.end(); ++it ) {
        QString thumbnail = QString::fromLatin1( "%1/ThumbNails/%2" ).arg(path).arg(*it);
        QDir().remove( thumbnail );
    }

}

bool Util::canReadImage( const QString& fileName )
{
    return KImageIO::canRead(KImageIO::type(fileName)) || isCRW( fileName );
}


QString Util::readFile( const QString& fileName )
{
    if ( fileName.isEmpty() ) {
        KMessageBox::error( 0, i18n("<qt>Unable to find file %1</qt>").arg( fileName ) );
        return QString::null;
    }

    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        //KMessageBox::error( 0, i18n("Could not open file %1").arg( fileName ) );
        return QString::null;
    }

    QTextStream stream( &file );
    QString content = stream.read();
    file.close();

    return content;
}

struct myjpeg_error_mgr : public jpeg_error_mgr
{
    jmp_buf setjmp_buffer;
};

extern "C"
{
    static void myjpeg_error_exit(j_common_ptr cinfo)
    {
        myjpeg_error_mgr* myerr =
            (myjpeg_error_mgr*) cinfo->err;

        char buffer[JMSG_LENGTH_MAX];
        (*cinfo->err->format_message)(cinfo, buffer);
        //kdWarning() << buffer << endl;
        longjmp(myerr->setjmp_buffer, 1);
    }
}

bool Util::loadJPEG(QImage *img, const QString& imageFile, QSize* fullSize, int width, int height)
{
    FILE* inputFile=fopen( QFile::encodeName(imageFile), "rb");
    if(!inputFile)
        return false;
    bool ok = loadJPEG( img, inputFile, fullSize, width, height );
    fclose(inputFile);
    return ok;
}

bool Util::loadJPEG(QImage *img, FILE* inputFile, QSize* fullSize, int width, int height)
{
    struct jpeg_decompress_struct    cinfo;
    struct myjpeg_error_mgr jerr;

    // JPEG error handling - thanks to Marcus Meissner
    cinfo.err             = jpeg_std_error(&jerr);
    cinfo.err->error_exit = myjpeg_error_exit;

    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inputFile);
    jpeg_read_header(&cinfo, TRUE);
    *fullSize = QSize( cinfo.image_width, cinfo.image_height );

    int imgSize = QMAX(cinfo.image_width, cinfo.image_height);

    //libjpeg supports a sort of scale-while-decoding which speeds up decoding
    int scale=1;
    if (width != -1 || height != -1) {
        int size_=QMAX(width, height);
        while(size_*scale*2<=imgSize) {
            scale*=2;
        }
        if(scale>8) scale=8;
    }

    cinfo.scale_num=1;
    cinfo.scale_denom=scale;

    // Create QImage
    jpeg_start_decompress(&cinfo);

    switch(cinfo.output_components) {
    case 3:
    case 4:
        if (!img->create( cinfo.output_width, cinfo.output_height, 32 ))
            return false;
        break;
    case 1: // B&W image
        if (!img->create( cinfo.output_width, cinfo.output_height,
                          8, 256 ))
            return false;
        for (int i=0; i<256; i++)
            img->setColor(i, qRgb(i,i,i));
        break;
    default:
        return false;
    }

    uchar** lines = img->jumpTable();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                            cinfo.output_height);
    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 ) {
        for (uint j=0; j<cinfo.output_height; j++) {
            uchar *in = img->scanLine(j) + cinfo.output_width*3;
            QRgb *out = (QRgb*)( img->scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) {
                in-=3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    /*int newMax = QMAX(cinfo.output_width, cinfo.output_height);
      int newx = size_*cinfo.output_width / newMax;
      int newy = size_*cinfo.output_height / newMax;*/

    jpeg_destroy_decompress(&cinfo);

    //image = img.smoothScale(newx,newy);
    return true;
}

bool Util::isJPEG( const QString& fileName )
{
    QString format= QString::fromLocal8Bit( QImageIO::imageFormat( fileName ) );
    return format == QString::fromLocal8Bit( "JPEG" );
}

/* Load embedded JPEG preview from Canon CRW "digital negative" */
bool Util::loadCRW(QImage *img, const QString& imageFile, QSize* fullSize, int width, int height)
{
    static const off_t thumb_block_start_offset = -18;

    /* We just find the offset of the JPEG inside the CRW
       file, fseek() and pass the FILE* on to the JPEG loader.
       The code is inspired by CRWInfo
       (http://neuemuenze.heim1.tu-clausthal.de/~sven/crwinfo/)

       Steffen Hansen <hansen@kde.org>
    */
    FILE* inputFile=fopen( QFile::encodeName(imageFile), "rb");
    if(!inputFile)
        return false;
    if( fseek( inputFile, thumb_block_start_offset, SEEK_END ) != 0 ) {
        fclose( inputFile );
        return false;
    }
    long int offset_buffer;
    if( fread( &offset_buffer, 4, 1, inputFile ) <= 0 ) {
        fclose( inputFile );
        return false;
    }
    off_t thumb_block_start = 26 + (off_t)offset_buffer;
    if( fseek( inputFile, thumb_block_start, SEEK_SET ) ) {
        fclose( inputFile );
        return false;
    }
    bool rc = loadJPEG( img, inputFile, fullSize, width, height );
    fclose(inputFile);
    return rc;
}

bool Util::isCRW( const QString& fileName )
{
    /* Really cheesy filetype detection, but for now it works */
    return fileName.endsWith( QString::fromLatin1("CRW"), false);
}

ImageInfoList Util::shuffle( ImageInfoList list )
{
    static bool init = false;
    if ( !init ) {
        QTime midnight( 0, 0, 0 );
        srand( midnight.secsTo(QTime::currentTime()) );
        init = true;
    }

    ImageInfoList result;

    while ( list.count() != 0 ) {
        int index = (int) ( (double)list.count()* rand()/((double)RAND_MAX) );
        result.append( list.at(index) );
        list.remove( list.at(index) );
    }
    return result;
}

/**
   Create a maping from original name with path to uniq name without:
   cd1/def.jpg      -> def.jpg
   cd1/abc/file.jpg -> file.jpg
   cd3/file.jpg     -> file-2.jpg
*/
Util::UniqNameMap Util::createUniqNameMap( const ImageInfoList& images, bool relative, const QString& destDir  )
{
    QMap<QString, QString> map;
    QMap<QString, QString> inverseMap;

    for( ImageInfoListIterator it( images ); *it; ++it ) {
        QString fullName = (*it)->fileName( relative );
        QString base = QFileInfo( fullName ).baseName();
        QString ext = QFileInfo( fullName ).extension();
        QString file = base + QString::fromLatin1( "." ) +  ext;
        if ( !destDir.isNull() )
            file = QString::fromLatin1("%1/%2").arg(destDir).arg(file);

        if ( inverseMap.contains( file ) || ( !destDir.isNull() && QFileInfo( file ).exists() ) ) {
            int i = 1;
            bool clash;
            do {
                file = QString::fromLatin1( "%1-%2.%3" ).arg( base ).arg( ++i ).arg( ext );
                if ( !destDir.isNull() )
                    file = QString::fromLatin1("%1/%2").arg(destDir).arg(file);

                clash = inverseMap.contains( file ) ||
                        ( !destDir.isNull() && QFileInfo( file ).exists() );
            } while ( clash );
        }

        QString relFile = file;
        if ( relative ) {
            Q_ASSERT( file.startsWith( Options::instance()->imageDirectory() ) );
            relFile = file.mid( Options::instance()->imageDirectory().length() );
            if ( relFile.startsWith( QString::fromLatin1( "/" ) ) )
                relFile = relFile.mid(1);
        }

        map.insert( fullName, relFile );
        inverseMap.insert( file, fullName );
    }

    return map;
}

QString Util::stripSlash( const QString& fileName )
{
    if ( fileName.endsWith( QString::fromLatin1( "/" ) ) )
        return fileName.left( fileName.length()-1);
    else
        return fileName;
}

QString Util::relativeFolderName( const QString& fileName)
{
    int index= fileName.findRev( '/', -1);
    if (index == -1)
        return i18n("(base folder)");
    else
        return fileName.left( index ) ;
}

bool Util::runningDemo()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    return args->isSet( "demo" );
}

void Util::deleteDemo()
{
    QString dir = QString::fromLatin1( "/tmp/kimdaba-demo-" ) + QString::fromLocal8Bit( getenv( "LOGNAME" ) );
    KURL url;
    url.setPath( dir );
    (void) KIO::NetAccess::del( dir, MainView::theMainView() );
}

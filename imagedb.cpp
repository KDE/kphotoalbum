/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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

#include "imagedb.h"
#include "showbusycursor.h"
#include "options.h"
#include <qfileinfo.h>
#include <qfile.h>
#include <qdir.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "util.h"

ImageDB* ImageDB::_instance = 0;

ImageDB::ImageDB()
{
}

int ImageDB::totalCount() const
{
    return _images.count();
}

void ImageDB::search( const ImageSearchInfo& info, int from, int to )
{
    ShowBusyCursor dummy;
    int c = count( info, true, from, to );
    emit searchCompleted();
    emit matchCountChange( from, to, c );
}

int ImageDB::count( const ImageSearchInfo& info )
{
    return count( info, false, -1, -1 );
}

int ImageDB::count( const ImageSearchInfo& info, bool makeVisible, int from, int to )
{
    int count = 0;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = const_cast<ImageSearchInfo&>(info).match( *it ); // PENDING(blackie) remove cast
        if ( match )
            ++count;
        match &= ( from != -1 && to != -1 && from <= count && count < to ) ||
                 ( from == -1 && to == -1 );
        if ( makeVisible )
            (*it)->setVisible( match );
    }
    return count;
}

ImageDB* ImageDB::instance()
{
    if ( _instance == 0 )
        _instance = new ImageDB();
    return _instance;
}

void ImageDB::load()
{
    checkForBackupFile();

    _images.clear();

    QString directory = Options::instance()->imageDirectory();
    if ( directory.isEmpty() )
        return;
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );


    // Load the information from the XML file.
    QDict<void> loadedFiles( 6301 /* a large prime */ );

    QString xmlFile = directory + QString::fromLatin1("/index.xml");
    if ( QFileInfo( xmlFile ).exists() )  {
        QFile file( xmlFile );
        if ( ! file.open( IO_ReadOnly ) )  {
            qWarning( "Couldn't read file %s",  xmlFile.latin1() );
        }
        else {
            QDomDocument doc;
            doc.setContent( &file );
            for ( QDomNode node = doc.documentElement().firstChild(); !node.isNull(); node = node.nextSibling() )  {
                QDomElement elm;
                if ( node.isElement() )
                    elm = node.toElement();
                else
                    continue;

                QString fileName = elm.attribute( QString::fromLatin1("file") );
                if ( fileName.isNull() )
                    qWarning( "Element did not contain a file attribute" );
                else if ( loadedFiles.find( fileName ) != 0 )
                    qWarning( "XML file contained image %s, more than ones - only first one will be loaded", fileName.latin1());
                else {
                    loadedFiles.insert( directory + QString::fromLatin1("/") + fileName,
                                        (void*)0x1 /* void pointer to nothing I never need the value,
                                                      just its existsance, must be != 0x0 though.*/ );
                    load( fileName, elm );
                }
            }
        }
    }

    loadExtraFiles( loadedFiles, directory );
}

void ImageDB::load( const QString& fileName, QDomElement elm )
{
    ImageInfo* info = new ImageInfo( fileName, elm );
    info->setVisible( false );
    ImageDB::instance()->images().append(info);
}

void ImageDB::loadExtraFiles( const QDict<void>& loadedFiles, QString directory )
{
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );

    QString imageDir = Options::instance()->imageDirectory();
    if ( imageDir.endsWith( QString::fromLatin1("/") ) )
        imageDir = imageDir.mid( 0, imageDir.length()-1 );

    QDir dir( directory );
    QStringList dirList = dir.entryList( QDir::All );
    for( QStringList::Iterator it = dirList.begin(); it != dirList.end(); ++it ) {
        QString file = directory + QString::fromLatin1("/") + *it;
        QFileInfo fi( file );
        if ( (*it) == QString::fromLatin1(".") || (*it) == QString::fromLatin1("..") ||
             (*it) == QString::fromLatin1("ThumbNails") || !fi.isReadable() )
                continue;

        // PENDING(blackie) Is there a way to ask KDE or Qt for available image extentions?
        if ( fi.isFile() && (loadedFiles.find( file ) == 0) &&
             ( (*it).endsWith( QString::fromLatin1(".jpg") ) ||
               (*it).endsWith( QString::fromLatin1(".jpeg") ) ||
               (*it).endsWith( QString::fromLatin1(".png") ) ||
               (*it).endsWith( QString::fromLatin1(".tiff") ) ||
               (*it).endsWith( QString::fromLatin1(".gif") ) ) )  {
            QString baseName = file.mid( imageDir.length()+1 );

            ImageInfo* info = new ImageInfo( baseName  );
            ImageDB::instance()->images().append(info);
        }
        else if ( fi.isDir() )  {
            loadExtraFiles( loadedFiles, file );
        }
    }
}

void ImageDB::checkForBackupFile()
{
    QString backupNm = Options::instance()->imageDirectory() + QString::fromLatin1("/.#index.xml");
    QString indexNm = Options::instance()->imageDirectory() + QString::fromLatin1("/index.xml");
    Util::checkForBackupFile( indexNm, backupNm );
}

void ImageDB::save( const QString& fileName )
{
    ShowBusyCursor dummy;

    ImageInfoList list = _images;

    // Copy files from clipboard to end of overview, so we don't loose them
    for( ImageInfoListIterator it(_clipboard); *it; ++it ) {
        list.append( *it );
    }

    // Open the output file
    QDomDocument doc;

    // PENDING(blackie) The user should be able to specify the coding himself.
    doc.appendChild( doc. createProcessingInstruction( QString::fromLatin1("xml"), QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );
    QDomElement elm = doc.createElement( QString::fromLatin1("images") );
    doc.appendChild( elm );

    for( ImageInfoListIterator it( list ); *it; ++it ) {
        elm.appendChild( (*it)->save( doc ) );
    }

    QFile out( fileName );

    if ( !out.open( IO_WriteOnly ) )  {
        qWarning( "Could not open file '%s'", fileName.latin1() );
    }
    else {
        QTextStream stream( &out );
        stream << doc.toString().utf8();
        out.close();
    }
}

bool ImageDB::isClipboardEmpty()
{
    return _clipboard.count() == 0;
}

QMap<QString,int> ImageDB::classify( const ImageSearchInfo& info, const QString &group )
{
    QMap<QString, int> map;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        bool match = const_cast<ImageSearchInfo&>(info).match( *it ); // PENDING(blackie) remove cast
        if ( match ) {
            QStringList list = (*it)->optionValue(group);
            for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
                map[*it]++;
            }
            if ( list.count() == 0 )
                map[i18n( "**NONE**" )]++;
        }
    }
    return map;
}






int ImageDB::countItemsOfOptionGroup( const QString& group )
{
    int count = 0;
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        if ( (*it)->optionValue( group ).count() != 0 )
            ++count;
    }
    return count;
}

void ImageDB::renameOptionGroup( const QString& oldName, const QString newName )
{
    for( ImageInfoListIterator it( _images ); *it; ++it ) {
        (*it)->renameOptionGroup( oldName, newName );
    }
}

#include "imagedb.moc"

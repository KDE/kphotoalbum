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
#include <kimageio.h>
#include "util.h"

ImageDB* ImageDB::_instance = 0;

ImageDB::ImageDB( const QDomElement& top, const QDomElement& blockList )
{
    QString directory = Options::instance()->imageDirectory();
    if ( directory.isEmpty() )
        return;
    if ( directory.endsWith( QString::fromLatin1("/") ) )
        directory = directory.mid( 0, directory.length()-1 );


    // Load the information from the XML file.
    QDict<void> loadedFiles( 6301 /* a large prime */ );

    for ( QDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() )  {
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

    // Read the block list
    for ( QDomNode node = blockList.firstChild(); !node.isNull(); node = node.nextSibling() )  {
        QDomElement elm;
        if ( node.isElement() )
            elm = node.toElement();
        else
            continue;

        QString fileName = elm.attribute( QString::fromLatin1( "file" ) );
        if ( !fileName.isEmpty() )
            _blockList << fileName;
    }

    loadExtraFiles( loadedFiles, directory );
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
        qFatal("ImageDB::instance must not be called before ImageDB::setup");
    return _instance;
}

void ImageDB::setup( const QDomElement& top, const QDomElement& blockList )
{
    _instance = new ImageDB( top, blockList );
}

void ImageDB::load( const QString& fileName, QDomElement elm )
{
    ImageInfo* info = new ImageInfo( fileName, elm );
    info->setVisible( false );
    images().append(info);
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

        if ( fi.isFile() && (loadedFiles.find( file ) == 0) &&
             KImageIO::canRead(KImageIO::type(fi.extension())) ) {
            QString baseName = file.mid( imageDir.length()+1 );

            if ( ! _blockList.contains( baseName ) ) {
                ImageInfo* info = new ImageInfo( baseName  );
                images().append(info);
            }
        }
        else if ( fi.isDir() )  {
            loadExtraFiles( loadedFiles, file );
        }
    }
}

void ImageDB::save( QDomElement top )
{
    ImageInfoList list = _images;

    // Copy files from clipboard to end of overview, so we don't loose them
    for( ImageInfoListIterator it(_clipboard); *it; ++it ) {
        list.append( *it );
    }

    QDomDocument doc = top.ownerDocument();
    QDomElement images = doc.createElement( QString::fromLatin1( "images" ) );
    top.appendChild( images );

    for( ImageInfoListIterator it( list ); *it; ++it ) {
        images.appendChild( (*it)->save( doc ) );
    }

    QDomElement blockList = doc.createElement( QString::fromLatin1( "blocklist" ) );
    bool any=false;
    for( QStringList::Iterator it = _blockList.begin(); it != _blockList.end(); ++it ) {
        any=true;
        QDomElement elm = doc.createElement( QString::fromLatin1( "block" ) );
        elm.setAttribute( QString::fromLatin1( "file" ), *it );
        blockList.appendChild( elm );
    }

    if (any)
        top.appendChild( blockList );
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

void ImageDB::blockList( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it) {
        _blockList << (*it)->fileName( true );
        _images.removeRef( *it );
    }
}

void ImageDB::deleteList( const ImageInfoList& list )
{
    for( ImageInfoListIterator it( list ); *it; ++it ) {
        _images.removeRef( *it );
    }
}

#include "imagedb.moc"

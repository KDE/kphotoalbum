/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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

#include "util.h"
#include "options.h"
#include "imageinfo.h"
#include <klocale.h>
#include <qfileinfo.h>
#include <kmessagebox.h>
#include <qurl.h>
#include <kapplication.h>
#include <unistd.h>
#include <qdir.h>
#include <kstandarddirs.h>
#include <stdlib.h>
#include <qregexp.h>
#include <kfilemetainfo.h>
#include <kfilemetainfo.h>

bool Util::writeOptions( QDomDocument doc, QDomElement elm, QMap<QString, QStringList>& options,
                         QMap<QString,Options::OptionGroupInfo>* optionGroupInfo )
{
    bool anyAtAll = false;
    QStringList grps = Options::instance()->optionGroups();
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        QString name = *it;
        opt.setAttribute( QString::fromLatin1("name"),  name );

        if ( optionGroupInfo ) {
            opt.setAttribute( QString::fromLatin1( "text" ), (*optionGroupInfo)[name]._text );
            opt.setAttribute( QString::fromLatin1( "icon" ), (*optionGroupInfo)[name]._icon );
            opt.setAttribute( QString::fromLatin1( "show" ), (*optionGroupInfo)[name]._show );
            opt.setAttribute( QString::fromLatin1( "viewsize" ), (*optionGroupInfo)[name]._size );
            opt.setAttribute( QString::fromLatin1( "viewtype" ), (*optionGroupInfo)[name]._type );
        }

        QStringList list = options[name];
        bool any = false;
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *it2 );
            opt.appendChild( val );
            any = true;
            anyAtAll = true;
        }
        if ( any || optionGroupInfo  ) // We always want to write all records when writing from Options
            elm.appendChild( opt );
    }
    return anyAtAll;
}



void Util::readOptions( QDomElement elm, QMap<QString, QStringList>* options,
                        QMap<QString,Options::OptionGroupInfo>* optionGroupInfo )
{
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "options" ) );

    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull();
          nodeOption = nodeOption.nextSibling() )  {

        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            Q_ASSERT( elmOption.tagName() == QString::fromLatin1("option") );
            QString name = elmOption.attribute( QString::fromLatin1("name") );
            if ( !name.isNull() )  {
                // Read Option Group info
                if ( optionGroupInfo ) {
                    QString text= elmOption.attribute( QString::fromLatin1("text"), name );
                    QString icon= elmOption.attribute( QString::fromLatin1("icon") );
                    Options::ViewSize size =
                        (Options::ViewSize) elmOption.attribute( QString::fromLatin1("viewsize"), QString::fromLatin1( "0" ) ).toInt();
                    Options::ViewType type =
                        (Options::ViewType) elmOption.attribute( QString::fromLatin1("viewtype"), QString::fromLatin1( "0" ) ).toInt();
                    bool show = (bool) elmOption.attribute( QString::fromLatin1( "show" ),
                                                            QString::fromLatin1( "1" ) ).toInt();
                    (*optionGroupInfo)[name] = Options::OptionGroupInfo( text, icon, size, type, show );
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

    QStringList grps = Options::instance()->optionGroups();
    int link = 0;
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QString optionGroup = *it;
        if ( Options::instance()->showOption( optionGroup ) ) {
            QStringList items = info->optionValue( optionGroup );
            if (items.count() != 0 ) {
                text += QString::fromLatin1( "<b>%1: </b> " )
                        .arg( Options::instance()->textForOptionGroup( optionGroup ) );
                bool first = true;
                for( QStringList::Iterator it2 = items.begin(); it2 != items.end(); ++it2 ) {
                    QString item = *it2;
                    if ( first )
                        first = false;
                    else
                        text += QString::fromLatin1( ", " );

                    if ( linkMap ) {
                        ++link;
                        (*linkMap)[link] = QPair<QString,QString>( optionGroup, item );
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
            KMessageBox::error( 0, i18n("Unable to create directory '%1' needed for demo").arg( dir ), i18n("Error running demo") );
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
    QFile out( configFile );
    if ( !out.open( IO_WriteOnly ) ) {
        KMessageBox::error( 0, i18n("Unable to open '%1' for writting").arg( configFile ), i18n("Error running demo") );
        exit(-1);
    }
    QTextStream( &out ) << str;
    out.close();

    // Images
    QStringList files = KStandardDirs().findAllResources( "data", QString::fromLatin1("kimdaba/demo/*.jpg" ) );
    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
        QString destFile = dir + QString::fromLatin1( "/" ) + QFileInfo(*it).fileName();
        if ( ! QFileInfo( destFile ).exists() ) {
            ok = ( symlink( (*it).latin1(), destFile.latin1() ) == 0 );
            if ( !ok ) {
                KMessageBox::error( 0, i18n("Unable to make symlink from '%1' to '%2'").arg( *it ).arg( destFile ), i18n("Error running demo") );
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
            KMessageBox::error( 0, i18n("Unable to create directory '%1' needed for demo").arg( dir ), i18n("Error running demo") );
            exit(-1);
        }
    }

    // Category images.
    files = KStandardDirs().findAllResources( "data", QString::fromLatin1("kimdaba/demo/CategoryImages/*.jpg" ) );
    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
        QString destFile = dir + QString::fromLatin1( "/" ) + QFileInfo(*it).fileName();
        if ( ! QFileInfo( destFile ).exists() ) {
            ok = ( symlink( (*it).latin1(), destFile.latin1() ) == 0 );
            if ( !ok ) {
                KMessageBox::error( 0, i18n("Unable to make symlink from '%1' to '%2'").arg( *it ).arg( destFile ), i18n("Error running demo") );
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

QString Util::readInstalledFile( const QString& fileName )
{
    QString inFileName = locate( "data", QString::fromLatin1( "kimdaba/%1" ).arg( fileName ) );
    if ( inFileName.isEmpty() ) {
        KMessageBox::error( 0, i18n("<qt>Couldn't find kimdaba/%1 - this is likely an installation error - Did you remember to do a make install, and did you set KDEDIRS, in case you did not install it in the default place.</qt>").arg( fileName ) );
        return QString::null;
    }

    QFile file( inFileName );
    if ( !file.open( IO_ReadOnly ) ) {
        KMessageBox::error( 0, i18n("Couldn't open file %s").arg( inFileName ) );
        return QString::null;
    }

    QTextStream stream( &file );
    QString content = stream.read();
    file.close();

    return content;
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

QString Util::readFile( const QString& fileName )
{
    if ( fileName.isEmpty() ) {
        KMessageBox::error( 0, i18n("<qt>Couldn't find file %1</qt>").arg( fileName ) );
        return QString::null;
    }

    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        //KMessageBox::error( 0, i18n("Couldn't open file %1").arg( fileName ) );
        return QString::null;
    }

    QTextStream stream( &file );
    QString content = stream.read();
    file.close();

    return content;
}

QMap<QString,QVariant> Util::getEXIF( const QString& fileName )
{
    QMap<QString,QVariant> map;
    KFileMetaInfo metainfo( fileName );
    if ( metainfo.isEmpty() )
        return map;

    QStringList keys = metainfo.supportedKeys();
    for( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
        KFileMetaInfoItem item = metainfo.item( *it );
        map.insert( *it, item.value() );
    }
    return map;
}

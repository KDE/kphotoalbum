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
                    bool show = (bool) elmOption.attribute( QString::fromLatin1( "show" ),
                                                            QString::fromLatin1( "1" ) ).toInt();
                    (*optionGroupInfo)[name] = Options::OptionGroupInfo( text, icon, show );
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
            text += info->startDate() + i18n(" to ") + info->endDate();

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

void Util::checkForBackupFile( const QString& realName, const QString& backupName )
{
    QFileInfo backUpFile( backupName);
    QFileInfo indexFile( realName );
    if ( !backUpFile.exists() || indexFile.lastModified() > backUpFile.lastModified() )
        return;

    int code = KMessageBox::questionYesNo( 0, i18n("Backup file '%1' exists and is newer than '%2'. "
                                                      "Should I use the backup file?")
                                           .arg(backupName).arg(realName),
                                           i18n("Found Backup File") );
    if ( code == KMessageBox::Yes ) {
        QFile in( backupName );
        if ( in.open( IO_ReadOnly ) ) {
            QFile out( realName );
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

bool Util::setupDemo()
{
    QString dir = QString::fromLatin1( "/tmp/kimdaba-demo-" ) + QString::fromLocal8Bit( getenv( "LOGNAME" ) );
    QFileInfo fi(dir);
    if ( ! fi.exists() ) {
        bool ok = QDir().mkdir( dir );
        if ( !ok ) {
            KMessageBox::error( 0, i18n("Unable to create directory '%1' needed for demo").arg( dir ), i18n("Error running demo") );
            return false;
        }
    }

    bool ok;

    // index.xml
    QString srcFile = locate( "data", QString::fromLatin1( "kimdaba/demo/index.xml" ) );
    QString destFile = dir + QString::fromLatin1( "/index.xml" );
    if ( ! QFileInfo( destFile ).exists() ) {
        ok = copy( srcFile, destFile );
        if ( !ok ) {
            KMessageBox::error( 0, i18n("Unable to copy '%1' to '%2'").arg( srcFile ).arg( destFile ), i18n("Error running demo") );
            return false;
        }
    }


    // setup
    srcFile = locate( "data", QString::fromLatin1( "kimdaba/demo/setup" ) );
    destFile = dir + QString::fromLatin1( "/setup" );

    QFile in( srcFile );
    ok = in.open( IO_ReadOnly );
    if ( !ok ) {
        KMessageBox::error( 0, i18n("Unable to open '%1' for reading").arg( srcFile ), i18n("Error running demo") );
        return false;
    }
    QString str = QTextStream( &in ).read();
    in.close();
    str = str.replace( QRegExp( QString::fromLatin1("imageDirectory=\"[^\"]*\"")), QString::fromLatin1("imageDirectory=\"%1\"").arg(dir) );
    str = str.replace( QRegExp( QString::fromLatin1("htmlBaseDir=\"[^\"]*\"")), QString::fromLatin1("") );
    str = str.replace( QRegExp( QString::fromLatin1("htmlBaseURL=\"[^\"]*\"")), QString::fromLatin1("") );
    QFile out( destFile );
    if ( !out.open( IO_WriteOnly ) ) {
        KMessageBox::error( 0, i18n("Unable to open '%1' for writting").arg( destFile ), i18n("Error running demo") );
        return false;
    }
    QTextStream( &out ) << str;
    out.close();

    // Images
    QStringList files = KStandardDirs().findAllResources( "data", QString::fromLatin1("kimdaba/demo/*.jpg" ) );
    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it ) {
        destFile = dir + QString::fromLatin1( "/" ) + QFileInfo(*it).fileName();
        if ( ! QFileInfo( destFile ).exists() ) {
            ok = ( symlink( (*it).latin1(), destFile.latin1() ) == 0 );
            if ( !ok ) {
                KMessageBox::error( 0, i18n("Unable to make symlink from '%1' to '%2'").arg( *it ).arg( destFile ), i18n("Error running demo") );
                return false;
            }
        }

    }
    Options::setConfFile( dir + QString::fromLatin1( "/setup" ) );
    return true;
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


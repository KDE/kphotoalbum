/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "XMLHandler.h"
#include <qdom.h>
#include "Utilities/Util.h"
#include "DB/ImageDB.h"

using Utilities::StringSet;

/**
 * \class ImportExport::XMLHandler
 * \brief Helper class for
 * reading and writting the index.xml file located in exported .kim file.
 * This class is a simple helper class which encapsulate the code needed for generating an index.xml for the export file.
 * There should never be a need to keep any instances around of this class, simply create one on the stack, and call
 * thee method \ref createIndexXML().
 *
 * Notice, you will find a lot of duplicated code inhere from the XML database, there are two reasons for this
 * (1) In the long run the XML database ought to be an optional part (users might instead use, say an SQL database)
 * (2) To ensure that the .kim files are compatible both forth and back between versions, I'd rather keep that code
 * separate from the normal index.xml file, which might change with KPhotoAlbum versions to e.g. support compression.
 */
QCString ImportExport::XMLHandler::createIndexXML( const QStringList& images, const QString& baseUrl,
                                                   ImageFileLocation location, const Utilities::UniqNameMap& nameMap )
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( QString::fromLatin1("xml"),
                                                      QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"") ) );

    QDomElement top = doc.createElement( QString::fromLatin1( "KimDaBa-export" ) ); // Don't change, as this will make the files unreadable for KimDaBa 2.1 and back.
    top.setAttribute( QString::fromLatin1( "location" ),
                      location == Inline ? QString::fromLatin1( "inline" ) : QString::fromLatin1( "external" ) );
    if ( !baseUrl.isEmpty() )
        top.setAttribute( QString::fromLatin1( "baseurl" ), baseUrl );
    doc.appendChild( top );


    for( QStringList::ConstIterator it = images.begin(); it != images.end(); ++it ) {
        QString mappedFile = nameMap[*it];
        QDomElement elm = save( doc, DB::ImageDB::instance()->info(*it) );
        elm.setAttribute( QString::fromLatin1( "file" ), mappedFile );
        elm.setAttribute( QString::fromLatin1( "angle" ), 0 ); // We have rotated the image while copying it
        top.appendChild( elm );
    }
    return doc.toCString();
}

QDomElement ImportExport::XMLHandler::save( QDomDocument doc, const DB::ImageInfoPtr& info )
{
    QDomElement elm = doc.createElement( QString::fromLatin1("image") );
    elm.setAttribute( QString::fromLatin1("label"),  info->label() );
    elm.setAttribute( QString::fromLatin1("description"), info->description() );

    DB::ImageDate date = info->date();
    QDateTime start = date.start();
    QDateTime end = date.end();

    elm.setAttribute( QString::fromLatin1("yearFrom"), start.date().year() );
    elm.setAttribute( QString::fromLatin1("monthFrom"),  start.date().month() );
    elm.setAttribute( QString::fromLatin1("dayFrom"),  start.date().day() );
    elm.setAttribute( QString::fromLatin1("hourFrom"), start.time().hour() );
    elm.setAttribute( QString::fromLatin1("minuteFrom"), start.time().minute() );
    elm.setAttribute( QString::fromLatin1("secondFrom"), start.time().second() );

    elm.setAttribute( QString::fromLatin1("yearTo"), end.date().year() );
    elm.setAttribute( QString::fromLatin1("monthTo"),  end.date().month() );
    elm.setAttribute( QString::fromLatin1("dayTo"),  end.date().day() );

    elm.setAttribute( QString::fromLatin1( "width" ), info->size().width() );
    elm.setAttribute( QString::fromLatin1( "height" ), info->size().height() );

    writeCategories( doc, elm, info );
    info->drawList().save( doc, elm );

    return elm;
}


void ImportExport::XMLHandler::writeCategories( QDomDocument doc, QDomElement root, const DB::ImageInfoPtr& info )
{
    QDomElement elm = doc.createElement( QString::fromLatin1("options") );

    bool anyAtAll = false;
    QStringList grps = info->availableCategories();
    for( QStringList::Iterator categoryIt = grps.begin(); categoryIt != grps.end(); ++categoryIt ) {
        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        QString name = *categoryIt;
        opt.setAttribute( QString::fromLatin1("name"),  name );

        StringSet items = info->itemsOfCategory(*categoryIt);
        bool any = false;
        for( StringSet::const_iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *itemIt );
            opt.appendChild( val );
            any = true;
            anyAtAll = true;
        }
        if ( any )
            elm.appendChild( opt );
    }

    if ( anyAtAll )
        root.appendChild( elm );
}

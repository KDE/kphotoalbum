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
#ifndef XMLDB_FILEREADER_H
#define XMLDB_FILEREADER_H

#include <qdom.h>
#include "DB/ImageInfoPtr.h"

namespace XMLDB
{
class Database;

class FileReader
{

public:
    FileReader( Database* db ) : _db( db ) {}
    void read( const QString& configFile );
    static QString unescape( const QString& );

protected:
    void readTopNodeInConfigDocument( const QString& configFile, QDomElement top, QDomElement* options, QDomElement* images,
                                      QDomElement* blockList, QDomElement* memberGroups );
    void loadCategories( const QDomElement& elm );
    void loadImages( const QDomElement& images );
    void loadBlockList( const QDomElement& blockList );
    void loadMemberGroups( const QDomElement& memberGroups );
    DB::ImageInfoPtr load( const QString& filename, QDomElement elm );
    QDomElement readConfigFile( const QString& configFile );

    void createSpecialCategories();

    void checkIfImagesAreSorted();
    void checkIfAllImagesHasSizeAttributes();
    void checkAndWarnAboutVersionConflict();

private:
    Database* _db;
    int _fileVersion;
};

}

#endif /* XMLDB_FILEREADER_H */


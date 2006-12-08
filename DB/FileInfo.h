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

#ifndef FILEINFO_H
#define FILEINFO_H

#include <qstring.h>
#include <qvariant.h>
#include <config.h>
#include <qdatetime.h>
#ifdef HASEXIV2
#  include "Exif/Info.h"
#endif

namespace DB
{

class FileInfo
{
public:
    static FileInfo read( const QString& fileName );
    QDateTime dateTime() { return _date; }
    int angle() { return _angle; };
    QString description() {return _description; }

protected:
#ifdef HASEXIV2
    void parseEXIV2( const QString& fileName );
    QDateTime fetchEXIV2Date( Exiv2::ExifData& map, const char* key );
#endif

    void parseKFileMetaInfo( const QString& fileName );
    int orientationToAngle( int orientation );

private:
    FileInfo( const QString& fileName );
    QDateTime _date;
    int _angle;
    QString _description;
};

}

#endif /* FILEINFO_H */


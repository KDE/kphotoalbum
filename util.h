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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef UTIL_H
#define UTIL_H
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include "options.h"
#include "imageinfolist.h"
class CategoryCollection;
class ImageInfo;

class Util {
public:
    static QString createInfoText( ImageInfoPtr info, QMap<int, QPair<QString,QString> >* );
    static void checkForBackupFile( const QString& fileName );
    static bool ctrlKeyDown();
    static bool copy( const QString& from, const QString& to );
    static bool makeHardLink( const QString& from, const QString& to );
    static bool runningDemo();
    static void deleteDemo();
    static QString setupDemo();
    static bool canReadImage( const QString& fileName );
    static QString readInstalledFile( const QString& fileName );
    static QString getThumbnailDir( const QString& imageFile );
    static QString getThumbnailFile( const QString& imageFile, int width, int height, int angle );
    static void removeThumbNail( const QString& imageFile );
    static QString readFile( const QString& fileName );
    static bool loadJPEG(QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1);
    static bool loadJPEG(QImage *img, FILE* inputFile, QSize* fullSize, int dim=-1);
    static bool isJPEG( const QString& fileName );

    static QStringList shuffle( const QStringList& list );

    typedef QMap<QString, QString> UniqNameMap;
    static UniqNameMap createUniqNameMap( const QStringList& images, bool relative, const QString& destdir );

    static QString stripSlash( const QString& fileName );
    static QString absoluteImageFileName( const QString& relativeName );
    static QString relativeFolderName( const QString& fileName);
    static QStringList infoListToStringList( const ImageInfoList& list );
    static QString stripImageDirectory( const QString& fileName );
    static QStringList diff( const QStringList& list1, const QStringList& list2 );
};

bool operator>( const QPoint&, const QPoint& );
bool operator<( const QPoint&, const QPoint& );

#endif /* UTIL_H */


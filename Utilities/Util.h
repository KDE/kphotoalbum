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

#ifndef UTIL_H
#define UTIL_H
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qimage.h>
#include "DB/ImageInfoList.h"
#include "DB/MD5.h"
#include <string>

namespace DB
{
    class ImageInfo;
    class CategoryCollection;
    class MD5;
}

namespace Utilities
{

QString createInfoText( DB::ImageInfoPtr info, QMap<int, QPair<QString,QString> >* );
void checkForBackupFile( const QString& fileName );
bool ctrlKeyDown();
bool copy( const QString& from, const QString& to );
void copyList( const QStringList& from, const QString& directoryTo );
bool makeHardLink( const QString& from, const QString& to );
bool runningDemo();
void deleteDemo();
QString setupDemo();
bool canReadImage( const QString& fileName );
bool isVideo( const QString& fileName );
QString getThumbnailDir( const QString& imageFile );
QString getThumbnailFile( const QString& imageFile, int width, int height, int angle );
void removeThumbNail( const QString& imageFile );
QString locateDataFile(const QString& fileName);
QString readFile( const QString& fileName );
bool loadJPEG(QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1);
bool isJPEG( const QString& fileName );

typedef QMap<QString, QString> UniqNameMap;
UniqNameMap createUniqNameMap( const QStringList& images, bool relative, const QString& destdir );

bool areSameFile( const QString fileName1, const QString fileName2 );
QString stripSlash( const QString& fileName );
QString absoluteImageFileName( const QString& relativeName );
QString relativeFolderName( const QString& fileName);
QStringList infoListToStringList( const DB::ImageInfoList& list );
QString stripImageDirectory( const QString& fileName );

QImage scaleImage(const QImage &image, int w, int h, QImage::ScaleMode mode=QImage::ScaleFree );
QImage scaleImage(const QImage &image, const QSize& s, QImage::ScaleMode mode=QImage::ScaleFree );

QString cStringWithEncoding( const char *c_str, const QString& charset );
QString cStringWithEncoding( const char *c_str,int charset );
std::string encodeQString( const QString& str, const QString& charset );
std::string encodeQString( const QString& str, int charset );
QStringList humanReadableCharsetList();

DB::MD5 MD5Sum( const QString& fileName );
};

bool operator>( const QPoint&, const QPoint& );
bool operator<( const QPoint&, const QPoint& );

#endif /* UTIL_H */


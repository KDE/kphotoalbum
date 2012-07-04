/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef UTIL_H
#define UTIL_H
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qimage.h>
#include "Settings/SettingsData.h"
#include "DB/ImageInfoList.h"
#include <stdio.h>
#include "DB/MD5.h"

namespace DB
{
    class ImageInfo;
}

namespace Utilities
{
QString createInfoText( DB::ImageInfoPtr info, QMap<int, QPair<QString,QString> >* );
void checkForBackupFile( const QString& fileName, const QString& message = QString() );
bool ctrlKeyDown();
bool copy( const QString& from, const QString& to );
void copyList( const QStringList& from, const QString& directoryTo );
bool makeSymbolicLink( const QString& from, const QString& to );
bool makeHardLink( const QString& from, const QString& to );
bool runningDemo();
void deleteDemo();
QString setupDemo();
bool canReadImage( const DB::FileName& fileName );
bool isVideo( const DB::FileName& fileName );
bool isRAW( const DB::FileName& fileName );
QString locateDataFile(const QString& fileName);
QString readFile( const QString& fileName );
bool loadJPEG(QImage *img, const DB::FileName& imageFile, QSize* fullSize, int dim=-1);
bool isJPEG( const DB::FileName& fileName );

QString stripEndingForwardSlash( const QString& fileName );

QString absoluteImageFileName( const QString& relativeName );
QString imageFileNameToAbsolute( const QString& fileName );

QString relativeFolderName( const QString& fileName);

QString stripImageDirectory( const QString& fileName );

QImage scaleImage(const QImage &image, int w, int h, Qt::AspectRatioMode mode=Qt::IgnoreAspectRatio );
QImage scaleImage(const QImage &image, const QSize& s, Qt::AspectRatioMode mode=Qt::IgnoreAspectRatio );

QString cStringWithEncoding( const char *c_str, const QString& charset );

DB::MD5 MD5Sum( const DB::FileName& fileName );

QColor contrastColor( const QColor& );

void saveImage( const DB::FileName& fileName, const QImage& image, const char* format );
}

bool operator>( const QPoint&, const QPoint& );
bool operator<( const QPoint&, const QPoint& );

#endif /* UTIL_H */


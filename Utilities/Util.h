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
#include "DB/CategoryPtr.h"
#include "DB/FileName.h"
#include "DB/ImageInfoPtr.h"

#include <QImage>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>

namespace Utilities
{
void checkForBackupFile( const QString& fileName, const QString& message = QString() );
bool copy( const QString& from, const QString& to );
bool makeSymbolicLink( const QString& from, const QString& to );
bool makeHardLink( const QString& from, const QString& to );
bool canReadImage( const DB::FileName& fileName );
const QSet<QString>& supportedVideoExtensions();
bool isVideo( const DB::FileName& fileName );
bool isRAW( const DB::FileName& fileName );
QString locateDataFile(const QString& fileName);
QString readFile( const QString& fileName );

QString stripEndingForwardSlash( const QString& fileName );

QString absoluteImageFileName( const QString& relativeName );
QString imageFileNameToAbsolute( const QString& fileName );

QString relativeFolderName( const QString& fileName);

QImage scaleImage(const QImage &image, int w, int h, Qt::AspectRatioMode mode=Qt::IgnoreAspectRatio );
QImage scaleImage(const QImage &image, const QSize& s, Qt::AspectRatioMode mode=Qt::IgnoreAspectRatio );

QString cStringWithEncoding( const char *c_str, const QString& charset );

QColor contrastColor( const QColor& );

void saveImage( const DB::FileName& fileName, const QImage& image, const char* format );
}


#endif /* UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

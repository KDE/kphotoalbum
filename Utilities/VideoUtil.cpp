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

#include "VideoUtil.h"

#include <QFileInfo>
#include <QSet>
#include <QString>

const QSet<QString>& Utilities::supportedVideoExtensions()
{
    static QSet<QString> videoExtensions;
    if ( videoExtensions.empty() ) {
        videoExtensions.insert( QString::fromLatin1( "3gp" ) );
        videoExtensions.insert( QString::fromLatin1( "avi" ) );
        videoExtensions.insert( QString::fromLatin1( "mp4" ) );
        videoExtensions.insert( QString::fromLatin1( "m4v" ) );
        videoExtensions.insert( QString::fromLatin1( "mpeg" ) );
        videoExtensions.insert( QString::fromLatin1( "mpg" ) );
        videoExtensions.insert( QString::fromLatin1( "qt" ) );
        videoExtensions.insert( QString::fromLatin1( "mov" ) );
        videoExtensions.insert( QString::fromLatin1( "moov" ) );
        videoExtensions.insert( QString::fromLatin1( "qtvr" ) );
        videoExtensions.insert( QString::fromLatin1( "rv" ) );
        videoExtensions.insert( QString::fromLatin1( "3g2" ) );
        videoExtensions.insert( QString::fromLatin1( "fli" ) );
        videoExtensions.insert( QString::fromLatin1( "flc" ) );
        videoExtensions.insert( QString::fromLatin1( "mkv" ) );
        videoExtensions.insert( QString::fromLatin1( "mng" ) );
        videoExtensions.insert( QString::fromLatin1( "asf" ) );
        videoExtensions.insert( QString::fromLatin1( "asx" ) );
        videoExtensions.insert( QString::fromLatin1( "wmp" ) );
        videoExtensions.insert( QString::fromLatin1( "wmv" ) );
        videoExtensions.insert( QString::fromLatin1( "ogm" ) );
        videoExtensions.insert( QString::fromLatin1( "rm" ) );
        videoExtensions.insert( QString::fromLatin1( "flv" ) );
        videoExtensions.insert( QString::fromLatin1( "webm" ) );
        videoExtensions.insert( QString::fromLatin1( "mts" ) );
        videoExtensions.insert( QString::fromLatin1( "ogg" ) );
        videoExtensions.insert( QString::fromLatin1( "ogv" ) );
        videoExtensions.insert( QString::fromLatin1( "m2ts" ) );
    }
    return videoExtensions;
}
bool Utilities::isVideo( const DB::FileName& fileName )
{
    QFileInfo fi( fileName.relative() );
    QString ext = fi.suffix().toLower();
    return supportedVideoExtensions().contains( ext );
}

// vi:expandtab:tabstop=4 shiftwidth=4:

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

#ifndef UTIL_H
#define UTIL_H
#include <qdom.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include "options.h"
class ImageInfo;

class Util {
public:
    static bool writeOptions( QDomDocument doc,  QDomElement elm, QMap<QString, QStringList>& options,
                              QMap<QString,Options::OptionGroupInfo>* optionGroupInfo );
    static void readOptions( QDomElement elm, QMap<QString, QStringList>* options,
                             QMap<QString,Options::OptionGroupInfo>* optionGroupInfo );
    static QString createInfoText( ImageInfo* info, QMap<int, QPair<QString,QString> >* );
    static void checkForBackupFile( const QString& fileName );
    static bool ctrlKeyDown();
    static bool copy( const QString& from, const QString& to );
    static bool runningDemo();
    static void deleteDemo();
    static QString setupDemo();
    static QString readInstalledFile( const QString& fileName );
    static QString getThumbnailDir( const QString& imageFile );
    static QString getThumbnailFile( const QString& imageFile, int width, int height, int angle );
    static void removeThumbNail( const QString& imageFile );
    static QString readFile( const QString& fileName );
    static QMap<QString,QVariant> getEXIF( const QString& fileName );
    static bool loadJPEG(QImage *img, const QString& imageFile, int width=-1, int height=-1);
    static bool isJPEG( const QString& fileName );
    
    static ImageInfoList shuffle( ImageInfoList list );

    typedef QMap<QString, QString> UniqNameMap;
    static UniqNameMap createUniqNameMap( const ImageInfoList& images, bool relative, const QString& destdir );

    static QString stripSlash( const QString& fileName );
};


#endif /* UTIL_H */


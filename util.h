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
    static QString setupDemo();

};


#endif /* UTIL_H */


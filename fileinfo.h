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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef FILEINFO_H
#define FILEINFO_H

#include <qmap.h>
#include <qstring.h>
#include <qvariant.h>

class FileInfo
{
public:
    static FileInfo read( const QString& fileName );
    bool isEmpty() const;
    QTime time( bool* foundTimeInExif = 0 ) const;
    QDate date( bool* foundDateInExif = 0 ) const;
    int angle( bool* found = 0 ) const;
    QString description( bool* found = 0 ) const;

private:
    QMap<QString,QVariant> _map;
    QString _fullPath;
};

#endif /* FILEINFO_H */


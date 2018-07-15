/* Copyright 2012 Jesper K. Pedersen <blackie@kde.org>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FILENAMELIST_H
#define FILENAMELIST_H

#include <QStringList>
#include <QList>
#include "FileName.h"
#include "ImageInfo.h"

namespace DB
{

class FileNameList : public QList<DB::FileName>
{
public:
    FileNameList() {}
    explicit FileNameList( const QList<DB::FileName>& );
    /**
     * @brief Create a FileNameList from a list of absolute filenames.
     * @param files
     */
    explicit FileNameList(const QStringList &files);
    QStringList toStringList(DB::PathType) const;
    FileNameList& operator<<(const DB::FileName& );
    FileNameList reversed() const;
};

}

#endif // FILENAMELIST_H
// vi:expandtab:tabstop=4 shiftwidth=4:

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

#include "FileNameList.h"

DB::FileNameList::FileNameList(const QList<DB::FileName>& other )
{
    QList<DB::FileName>::operator=(other);
}

DB::FileNameList::FileNameList(const QStringList &files)
{
    for (const QString& file: files)
        append(DB::FileName::fromAbsolutePath(file));
}

QStringList DB::FileNameList::toStringList(DB::PathType type) const
{
    QStringList res;

    for (const DB::FileName& fileName : *this) {
        if ( type == DB::RelativeToImageRoot )
            res.append( fileName.relative() );
        else
            res.append( fileName.absolute());
    }
    return res;
}

DB::FileNameList &DB::FileNameList::operator <<(const DB::FileName & fileName)
{
    QList<DB::FileName>::operator<<(fileName);
    return *this;
}

DB::FileNameList DB::FileNameList::reversed() const
{
    FileNameList res;
    for (const FileName& fileName : *this) {
        res.prepend(fileName);
    }
    return res;
}
// vi:expandtab:tabstop=4 shiftwidth=4:

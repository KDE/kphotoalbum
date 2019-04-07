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

#ifndef FILENAME_H
#define FILENAME_H

#include <QString>
#include <QSet>
#include "ImageInfoPtr.h"
#include <QtCore/qmetatype.h>

namespace DB
{

class FileName
{
public:
    FileName();
    static FileName fromAbsolutePath( const QString& fileName );
    static FileName fromRelativePath( const QString& fileName );
    QString absolute() const;
    QString relative() const;
    bool isNull() const;
    bool operator==( const FileName& other ) const;
    bool operator!=( const FileName& other ) const;
    bool operator<( const FileName& other ) const;
    bool exists() const;
    ImageInfoPtr info() const;

private:
    // During previous profilation it showed that converting between absolute and relative took quite some time,
    // so to avoid that, I store both.
    QString m_relativePath;
    QString m_absoluteFilePath;
    bool m_isNull;
};

uint qHash( const DB::FileName& fileName );
typedef QSet<DB::FileName> FileNameSet;
}

Q_DECLARE_METATYPE(DB::FileName)


#endif // FILENAME_H
// vi:expandtab:tabstop=4 shiftwidth=4:

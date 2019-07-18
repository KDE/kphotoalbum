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
#ifndef KIMFILEREADER_H
#define KIMFILEREADER_H
#include <QPixmap>
#include <QString>
class KArchiveDirectory;
class KZip;

namespace ImportExport
{

class KimFileReader
{
public:
    KimFileReader();
    ~KimFileReader();
    bool open(const QString &fileName);
    QByteArray indexXML();
    QPixmap loadThumbnail(QString fileName);
    QByteArray loadImage(const QString &fileName);

private:
    QString m_fileName;
    KZip *m_zip;
    const KArchiveDirectory *m_dir;
};

}

#endif /* KIMFILEREADER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

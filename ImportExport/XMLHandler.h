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
#ifndef XMLHANDLER_H
#define XMLHANDLER_H

#include <QDomDocument>
#include <QDomElement>
#include <QString>

#include <DB/FileNameList.h>
#include <DB/ImageInfoPtr.h>

#include "Export.h" // ImageFileLocation

namespace Utilities
{
class UniqFilenameMapper;
}

namespace ImportExport
{
class XMLHandler
{
public:
    QByteArray createIndexXML(
        const DB::FileNameList &images,
        const QString &baseUrl,
        ImageFileLocation location,
        Utilities::UniqFilenameMapper *nameMap);

protected:
    QDomElement save(QDomDocument doc, const DB::ImageInfoPtr &info);
    void writeCategories(QDomDocument doc, QDomElement elm, const DB::ImageInfoPtr &info);
};

}

#endif /* XMLHANDLER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

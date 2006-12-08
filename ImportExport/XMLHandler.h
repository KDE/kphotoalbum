/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef XMLHANDLER_H
#define XMLHANDLER_H
#include <qcstring.h>
#include <qstringlist.h>
#include <qstring.h>
#include "Export.h"
#include "Utilities/Util.h"
#include "DB/ImageInfoPtr.h"
#include <qdom.h>

namespace ImportExport
{
class XMLHandler
{
public:
    QCString createIndexXML( const QStringList& images, const QString& baseUrl, ImageFileLocation location,
                             const Utilities::UniqNameMap& nameMap );

protected:
    QDomElement save( QDomDocument doc, const DB::ImageInfoPtr& info );
    void writeCategories( QDomDocument doc, QDomElement elm, const DB::ImageInfoPtr& info );
};

}

#endif /* XMLHANDLER_H */


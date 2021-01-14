/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef XMLHANDLER_H
#define XMLHANDLER_H

#include "Export.h" // ImageFileLocation

#include <DB/ImageInfoPtr.h>
#include <kpabase/FileNameList.h>

#include <QDomDocument>
#include <QDomElement>
#include <QString>

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

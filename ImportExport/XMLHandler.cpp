/* Copyright (C) 2003-2020 The KPhotoAlbum Development Team

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
#include "XMLHandler.h"

#include <DB/FileName.h>
#include <DB/ImageDB.h>
#include <Utilities/FileUtil.h>

using Utilities::StringSet;

/**
 * \class ImportExport::XMLHandler
 * \brief Helper class for
 * reading and writing the index.xml file located in exported .kim file.
 * This class is a simple helper class which encapsulate the code needed for generating an index.xml for the export file.
 * There should never be a need to keep any instances around of this class, simply create one on the stack, and call
 * thee method \ref createIndexXML().
 *
 * Notice, you will find a lot of duplicated code inhere from the XML database, there are two reasons for this
 * (1) In the long run the XML database ought to be an optional part (users might instead use, say an SQL database)
 * (2) To ensure that the .kim files are compatible both forth and back between versions, I'd rather keep that code
 * separate from the normal index.xml file, which might change with KPhotoAlbum versions to e.g. support compression.
 */
QByteArray ImportExport::XMLHandler::createIndexXML(
    const DB::FileNameList &images,
    const QString &baseUrl,
    ImageFileLocation location,
    Utilities::UniqFilenameMapper *nameMap)
{
    QDomDocument doc;
    doc.appendChild(doc.createProcessingInstruction(QString::fromLatin1("xml"),
                                                    QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"")));

    QDomElement top = doc.createElement(QString::fromLatin1("KimDaBa-export")); // Don't change, as this will make the files unreadable for KimDaBa 2.1 and back.
    top.setAttribute(QString::fromLatin1("location"),
                     location == Inline ? QString::fromLatin1("inline") : QString::fromLatin1("external"));
    if (!baseUrl.isEmpty())
        top.setAttribute(QString::fromLatin1("baseurl"), baseUrl);
    doc.appendChild(top);

    for (const DB::FileName &fileName : images) {
        const QString mappedFile = nameMap->uniqNameFor(fileName);
        const auto info = DB::ImageDB::instance()->info(fileName);
        QDomElement elm = save(doc, info);
        elm.setAttribute(QString::fromLatin1("file"), mappedFile);
        top.appendChild(elm);
    }
    return doc.toByteArray();
}

QDomElement ImportExport::XMLHandler::save(QDomDocument doc, const DB::ImageInfoPtr &info)
{
    QDomElement elm = doc.createElement(QString::fromLatin1("image"));
    elm.setAttribute(QString::fromLatin1("label"), info->label());
    elm.setAttribute(QString::fromLatin1("description"), info->description());

    DB::ImageDate date = info->date();
    Utilities::FastDateTime start = date.start();
    Utilities::FastDateTime end = date.end();

    elm.setAttribute(QString::fromLatin1("yearFrom"), start.date().year());
    elm.setAttribute(QString::fromLatin1("monthFrom"), start.date().month());
    elm.setAttribute(QString::fromLatin1("dayFrom"), start.date().day());
    elm.setAttribute(QString::fromLatin1("hourFrom"), start.time().hour());
    elm.setAttribute(QString::fromLatin1("minuteFrom"), start.time().minute());
    elm.setAttribute(QString::fromLatin1("secondFrom"), start.time().second());

    elm.setAttribute(QString::fromLatin1("yearTo"), end.date().year());
    elm.setAttribute(QString::fromLatin1("monthTo"), end.date().month());
    elm.setAttribute(QString::fromLatin1("dayTo"), end.date().day());

    elm.setAttribute(QString::fromLatin1("width"), info->size().width());
    elm.setAttribute(QString::fromLatin1("height"), info->size().height());
    elm.setAttribute(QString::fromLatin1("md5sum"), info->MD5Sum().toHexString());
    elm.setAttribute(QString::fromLatin1("angle"), info->angle());

    writeCategories(doc, elm, info);

    return elm;
}

void ImportExport::XMLHandler::writeCategories(QDomDocument doc, QDomElement root, const DB::ImageInfoPtr &info)
{
    QDomElement elm = doc.createElement(QString::fromLatin1("options"));

    bool anyAtAll = false;
    const QStringList grps = info->availableCategories();
    for (const QString &name : grps) {
        QDomElement opt = doc.createElement(QString::fromLatin1("option"));
        opt.setAttribute(QString::fromLatin1("name"), name);

        const StringSet items = info->itemsOfCategory(name);
        bool any = false;
        for (const QString &item : items) {
            QDomElement val = doc.createElement(QString::fromLatin1("value"));
            val.setAttribute(QString::fromLatin1("value"), item);
            opt.appendChild(val);
            any = true;
            anyAtAll = true;
        }
        if (any)
            elm.appendChild(opt);
    }

    if (anyAtAll)
        root.appendChild(elm);
}
// vi:expandtab:tabstop=4 shiftwidth=4:

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
#ifndef IMPORTSETTINGS_H
#define IMPORTSETTINGS_H

#include "DB/ImageInfoList.h"
#include <QUrl>
namespace ImportExport
{

class CategoryMatchSetting
{
public:
    CategoryMatchSetting(const QString &DBCategoryName, const QString &XMLFileCategoryName)
        : m_XMLCategoryName(XMLFileCategoryName)
        , m_DBCategoryName(DBCategoryName)
    {
    }
    void add(const QString &DBFileNameItem, const QString &XMLFileNameItem);

    QString XMLCategoryName() const;
    QString DBCategoryName() const;
    const QMap<QString, QString> &XMLtoDB() const;

private:
    QString m_XMLCategoryName;
    QString m_DBCategoryName;
    QMap<QString, QString> m_XMLtoDB;
};

/**
 * The class contains all the data that is transported between the
 * ImportDialog, and the ImportHandler. The purpose of this class is to
 * decouple the above two.
 */
class ImportSettings
{
public:
    enum ImportAction { Replace = 1,
                        Keep = 2,
                        Merge = 3 };

    void setSelectedImages(const DB::ImageInfoList &);
    DB::ImageInfoList selectedImages() const;

    void setDestination(const QString &);
    QString destination() const;

    void setExternalSource(bool b);
    bool externalSource() const;

    void setKimFile(const QUrl &kimFile);
    QUrl kimFile() const;

    void setBaseURL(const QUrl &url);
    QUrl baseURL() const;

    void setImportActions(const QMap<QString, ImportAction> &actions);
    ImportAction importAction(const QString &item);

    void addCategoryMatchSetting(const CategoryMatchSetting &);
    QList<CategoryMatchSetting> categoryMatchSetting() const;

private:
    DB::ImageInfoList m_selectedImages;
    QString m_destination;
    bool m_externalSource;
    QUrl m_kimFile;
    QUrl m_baseURL;
    QMap<QString, ImportAction> m_actions;
    QList<CategoryMatchSetting> m_categoryMatchSettings;
};

}

#endif /* IMPORTSETTINGS_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

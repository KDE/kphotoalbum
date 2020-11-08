/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "ImportSettings.h"

void ImportExport::CategoryMatchSetting::add(const QString &DBFileNameItem, const QString &XMLFileNameItem)
{
    m_XMLtoDB[XMLFileNameItem] = DBFileNameItem;
}
void ImportExport::ImportSettings::setSelectedImages(const DB::ImageInfoList &list)
{
    m_selectedImages = list;
}

DB::ImageInfoList ImportExport::ImportSettings::selectedImages() const
{
    return m_selectedImages;
}

void ImportExport::ImportSettings::setDestination(const QString &destination)
{
    m_destination = destination;
    // makes appending easier:
    if (!m_destination.endsWith(QChar::fromLatin1('/')))
        m_destination.append(QChar::fromLatin1('/'));
}

QString ImportExport::ImportSettings::destination() const
{
    return m_destination;
}

void ImportExport::ImportSettings::setExternalSource(bool b)
{
    m_externalSource = b;
}

bool ImportExport::ImportSettings::externalSource() const
{
    return m_externalSource;
}

void ImportExport::ImportSettings::setKimFile(const QUrl &kimFile)
{
    m_kimFile = kimFile;
}

QUrl ImportExport::ImportSettings::kimFile() const
{
    return m_kimFile;
}

void ImportExport::ImportSettings::setBaseURL(const QUrl &url)
{
    m_baseURL = url;
}

QUrl ImportExport::ImportSettings::baseURL() const
{
    return m_baseURL;
}

ImportExport::ImportSettings::ImportAction ImportExport::ImportSettings::importAction(const QString &item)
{
    return m_actions[item];
}

void ImportExport::ImportSettings::setImportActions(const QMap<QString, ImportAction> &actions)
{
    m_actions = actions;
}

void ImportExport::ImportSettings::addCategoryMatchSetting(const CategoryMatchSetting &setting)
{
    m_categoryMatchSettings.append(setting);
}

QList<ImportExport::CategoryMatchSetting> ImportExport::ImportSettings::categoryMatchSetting() const
{
    return m_categoryMatchSettings;
}

QString ImportExport::CategoryMatchSetting::XMLCategoryName() const
{
    return m_XMLCategoryName;
}

QString ImportExport::CategoryMatchSetting::DBCategoryName() const
{
    return m_DBCategoryName;
}

const QMap<QString, QString> &ImportExport::CategoryMatchSetting::XMLtoDB() const
{
    return m_XMLtoDB;
}

// vi:expandtab:tabstop=4 shiftwidth=4:

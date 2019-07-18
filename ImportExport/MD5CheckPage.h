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
#ifndef MD5CHECKPAGE_H
#define MD5CHECKPAGE_H

#include "ImportSettings.h"
#include "Utilities/StringSet.h"
#include <QGridLayout>
#include <QWidget>
class QButtonGroup;

namespace ImportExport
{

class ClashInfo
{
public:
    explicit ClashInfo(const QStringList &categories);
    bool anyClashes();
    bool label;
    bool description;
    bool orientation;
    bool date;
    QMap<QString, bool> categories;
};

class MD5CheckPage : public QWidget
{
public:
    explicit MD5CheckPage(const ImportSettings &settings);
    static bool pageNeeded(const ImportSettings &settings);
    QMap<QString, ImportSettings::ImportAction> settings();

private:
    void createRow(QGridLayout *layout, int &row, const QString &name, const QString &title, bool anyClashes, bool allowMerge);
    static int countOfMD5Matches(const ImportSettings &settings);
    static ClashInfo clashes(const ImportSettings &settings);
    static Utilities::StringSet mapCategoriesToDB(const CategoryMatchSetting &matcher, const Utilities::StringSet &items);

private:
    QMap<QString, QButtonGroup *> m_groups;
};

}

#endif /* MD5CHECKPAGE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

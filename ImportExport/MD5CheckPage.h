/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MD5CHECKPAGE_H
#define MD5CHECKPAGE_H

#include "ImportSettings.h"

#include <Utilities/StringSet.h>

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

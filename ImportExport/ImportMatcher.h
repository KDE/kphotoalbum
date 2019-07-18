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

#ifndef IMPORTMATCHER_H
#define IMPORTMATCHER_H

#include <QScrollArea>

class QGridLayout;
class QComboBox;
class QCheckBox;

namespace ImportExport
{
class CategoryMatchSetting;

/**
 * This class encaptualte a single row in an ImportMatcher.
 */
class CategoryMatch
{
public:
    CategoryMatch(bool allowNew, const QString &categort, QStringList items, QWidget *parent, QGridLayout *grid, int row);
    QCheckBox *m_checkbox;
    QComboBox *m_combobox;
    QString m_text;
};

/**
 * This class is the configuration page for chooseing the matching between
 * the .kim file and the DB's items (e.g. .kim says People, DB say
 * Persons).
 */
class ImportMatcher : public QScrollArea
{
    Q_OBJECT

public:
    ImportMatcher(const QString &otherCategory, const QString &myCategory,
                  const QStringList &otherItems, const QStringList &myItems,
                  bool allowNew, QWidget *parent);
    CategoryMatchSetting settings();

    QString m_otherCategory;
    QString m_myCategory;
    QList<CategoryMatch *> m_matchers;
};

typedef QList<ImportMatcher *> ImportMatchers;
}

#endif /* IMPORTMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

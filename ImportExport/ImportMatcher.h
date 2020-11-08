/* SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
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

// SPDX-FileCopyrightText: 2003 - 2010 Jesper K. Pedersen <blackie@kde.org>
// SPDX-FileCopyrightText: 2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BROWSER_ENUMS_H
#define BROWSER_ENUMS_H

#include <Qt>

namespace Browser
{

constexpr int ItemNameRole = Qt::UserRole + 1;
constexpr int ValueRole = Qt::UserRole + 2;
constexpr int SortPriorityRole = Qt::UserRole + 3; ///< int, can be used to define "sort classes". I.e. items with the same priority are sorted alphabetically, but lower priorities are always sorted before higher priorities.
}

#endif /* BROWSER_ENUMS_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

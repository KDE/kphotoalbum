// SPDX-FileCopyrightText: 2003-2010 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2013 Dominik Broj <broj.dominik@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef DEMOUTIL_H
#define DEMOUTIL_H
#include <QString>

namespace Utilities
{
/**
 * @brief Delete the demo database.
 */
void deleteDemo();
/**
 * @brief Set up the demo database and return its location.
 * If anything goes wrong, a message is displayed and the program is terminated.
 * If there already is a database file at the demo location, load that one instead of copying the demo database.
 * @return the file name of index.xml
 */
QString setupDemo();
}

#endif /* DEMOUTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

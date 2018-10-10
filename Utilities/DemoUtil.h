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
 * @return the file name of index.xml
 */
QString setupDemo();
}

#endif /* DEMOUTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

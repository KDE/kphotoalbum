/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

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

#ifndef UTIL_H
#define UTIL_H

#include <QString>

/**
 * \file Utility functions for copying and linking files.
 *
 * \see FileNameUtils.h
 */

namespace Utilities
{
bool copyOrOverwrite( const QString& from, const QString& to );
bool makeSymbolicLink( const QString& from, const QString& to );
bool makeHardLink( const QString& from, const QString& to );

}


#endif /* UTIL_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

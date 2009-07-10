/* Copyright (C) 2003-2009 Jesper K. Pedersen <blackie@kde.org>

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

#ifndef ICON3DEFFECTS_H
#define ICON3DEFFECTS_H

#include <QPixmap>

namespace Utilities
{

/**
 * \brief Simple utility class, which adds 3D effects to pixmaps
 *
 * To make the pixmaps more attractive in the thumbnail viewer, their
 * corners are rounded, and a drop shadow are added.
 *
 * This class does exactly that.
 */
class Icon3DEffects
{
public:
    static QPixmap addEffects( const QPalette& palette, const QPixmap& pixmap );
private:
    static QPixmap emptyPixmapTemplate( const QPalette& palette, const QSize& size );
    static int borderSizeForRoundedPixmap( const QSize& size );
};

}

#endif /* ICON3DEFFECTS_H */


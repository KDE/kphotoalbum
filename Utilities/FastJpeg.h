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

#ifndef FASTJPEG_H
#define FASTJPEG_H
#include <QString>
#include <QImage>

namespace DB
{
class FileName;
}

namespace Utilities
{
bool loadJPEG(QImage *img, const DB::FileName& imageFile, QSize* fullSize, int dim=-1);
bool loadJPEG(QImage *img, const DB::FileName& imageFile, QSize* fullSize, int dim=-1, char *membuf=NULL, size_t membufSize=0);
bool loadJPEG(QImage *img, const QByteArray& data, QSize* fullSize, int dim=-1);
bool isJPEG( const DB::FileName& fileName );
}

#endif /* FASTJPEG_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

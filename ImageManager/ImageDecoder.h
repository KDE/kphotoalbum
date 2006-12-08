/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef IMAGEDECODER_H
#define IMAGEDECODER_H

#include <qptrlist.h>
#include <qimage.h>

namespace ImageManager
{

class ImageDecoder {
public:
	static bool decode( QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1 );
	static bool mightDecode( const QString& imageFile );

	virtual ~ImageDecoder();

protected:
	ImageDecoder();
	virtual bool _decode( QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1 ) = 0;
	virtual bool _mightDecode( const QString& imageFile ) = 0;

private:
	static QPtrList<ImageDecoder>* decoders();
};
}

#endif /* IMAGEDECODER_H */


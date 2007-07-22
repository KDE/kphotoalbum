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
#ifndef RAWIMAGEDECODER_H
#define RAWIMAGEDECODER_H

#include "ImageDecoder.h"
#include <qdict.h>

namespace ImageManager
{

class RAWImageDecoder : public ImageDecoder {
public:
	RAWImageDecoder() { _initializeExtensionLists(); }

	virtual bool _decode(QImage *img, const QString& imageFile, QSize* fullSize, int dim=-1);
	virtual bool _mightDecode( const QString& imageFile );
	virtual bool _skipThisFile( const QDict<void>& loadedFiles, const QString& imageFile );

private:
	bool _fileExistsWithExtensions( const QString& fileName, const QStringList& extensionList ) const;
	bool _fileEndsWithExtensions( const QString& fileName, const QStringList& extensionList ) const;
	bool _fileIsKnownWithExtensions( const QDict<void>& files, const QString& fileName, const QStringList& extensionList ) const;

	static QStringList _rawExtensions;
	static QStringList _standardExtensions;
	static QStringList _ignoredExtensions;
	static void _initializeExtensionLists();
};

}

#endif /* RAWIMAGEDECODER_H */

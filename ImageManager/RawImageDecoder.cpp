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
#include "RawImageDecoder.h"

#include <ktempfile.h>
#include <qfile.h>
#include <qimage.h>
#include <qwmatrix.h>
#include "Settings/SettingsData.h"

/* Main entry point into raw parser */
extern "C" {
	int extract_thumbnail( FILE*, FILE*, int* );
}

namespace ImageManager
{

bool RAWImageDecoder::_decode( QImage *img, const QString& imageFile, QSize* fullSize, int dim)
{
  /* width and height seem to be only hints, ignore */
  Q_UNUSED( dim );
  /* Open file and extract thumbnail */
  FILE* input = fopen( QFile::encodeName(imageFile), "rb" );
  if( !input ) return false;
  KTempFile output;
  output.setAutoDelete(true);
  int orientation = 0;
  if( extract_thumbnail( input, output.fstream(), &orientation ) ) {
	fclose(input);
	return false;
  }
  fclose(input);
  output.close();
  if( !img->load( output.name() ) ) return false;

  if( fullSize ) *fullSize = img->size();

  return true;
}

bool RAWImageDecoder::_mightDecode( const QString& imageFile )
{
	/* Known RAW file extensions. TODO: Complete */
	static const QString extensions[] = { QString::fromLatin1("crw"),
										  QString::fromLatin1("cr2"),
										  QString::fromLatin1("nef"),
										  QString::fromLatin1("bay"),
										  QString::fromLatin1("mos"),
										  QString::fromLatin1("mrw"),
										  QString::fromLatin1("orf"),
										  QString::fromLatin1("cs1"),
										  QString::fromLatin1("dc2"),
										  QString::fromLatin1("kdc"),
										  QString::fromLatin1("raf"),
										  QString::fromLatin1("rdc"),
										  QString::fromLatin1("x3f"),
										  QString::null };
	if (Settings::SettingsData::instance()->dontReadRawFilesWithOtherMatchingFile()) {
            static const QString standardExtensions[] = {
                QString::fromLatin1("jpg"),
                QString::fromLatin1("JPG"),
                QString::fromLatin1("tif"),
                QString::fromLatin1("TIF"),
                QString::fromLatin1("png"),
                QString::fromLatin1("PNG"),
                QString::null };
            QString baseFileName = imageFile;
            baseFileName.remove(baseFileName.length() - 3, 3);
            for (int i = 0; !standardExtensions[i].isNull(); ++i) {
                if (QFile::exists(baseFileName + standardExtensions[i]))
                    return false;
            }
	}

	for( int i = 0; !extensions[i].isNull(); ++i ) {
		if( imageFile.endsWith( extensions[i], false ) ) return true;
	}
	return false;
}

}

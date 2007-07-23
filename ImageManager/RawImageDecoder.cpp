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

#ifdef TEMPORARILY_REMOVED
#include <ktempfile.h>
#endif
#include <qfile.h>
#include <qimage.h>
#include <qmatrix.h>
#include <qstringlist.h>
#include "Settings/SettingsData.h"
#include <kdebug.h>

/* Main entry point into raw parser */
extern "C" {
	int extract_thumbnail( FILE*, FILE*, int* );
}

namespace ImageManager
{

bool RAWImageDecoder::_decode( QImage *img, const QString& imageFile, QSize* fullSize, int dim)
{
#ifdef TEMPORARILY_REMOVED
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
#else
  kDebug() << "TEMPORARILY REMOVED: " << k_funcinfo << endl;
#endif
}

QStringList RAWImageDecoder::_rawExtensions;
QStringList RAWImageDecoder::_standardExtensions;
QStringList RAWImageDecoder::_ignoredExtensions;

void RAWImageDecoder::_initializeExtensionLists()
{
  static bool extensionListsInitialized = 0;
  if (! extensionListsInitialized) {
    /* Known RAW file extensions. TODO: Complete */
    _rawExtensions << QString::fromLatin1("crw")
		   << QString::fromLatin1("cr2")
		   << QString::fromLatin1("nef")
		   << QString::fromLatin1("bay")
		   << QString::fromLatin1("mos")
		   << QString::fromLatin1("mrw")
		   << QString::fromLatin1("orf")
		   << QString::fromLatin1("cs1")
		   << QString::fromLatin1("dc2")
		   << QString::fromLatin1("kdc")
		   << QString::fromLatin1("raf")
		   << QString::fromLatin1("rdc")
		   << QString::fromLatin1("x3f")
		   << QString::fromLatin1("erf")
		   << QString::fromLatin1("pef");
    _standardExtensions << QString::fromLatin1("jpg")
			<< QString::fromLatin1("JPG")
			<< QString::fromLatin1("jpeg")
			<< QString::fromLatin1("JPEG")
			<< QString::fromLatin1("tif")
			<< QString::fromLatin1("TIF")
			<< QString::fromLatin1("tiff")
			<< QString::fromLatin1("TIFF")
			<< QString::fromLatin1("png")
			<< QString::fromLatin1("PNG");
    _ignoredExtensions << QString::fromLatin1("thm") // Thumbnails
		       << QString::fromLatin1("THM")
		       << QString::fromLatin1("thumb") // thumbnail files
						       // from dcraw
		       << QString::fromLatin1("ctg") // Catalog files
		       << QString::fromLatin1("gz") // Compressed files
		       << QString::fromLatin1("Z")
		       << QString::fromLatin1("bz2")
		       << QString::fromLatin1("zip")
		       << QString::fromLatin1("xml")
		       << QString::fromLatin1("XML")
		       << QString::fromLatin1("html")
		       << QString::fromLatin1("HTML")
		       << QString::fromLatin1("htm")
		       << QString::fromLatin1("HTM");
    extensionListsInitialized = 1;
  }
}

bool RAWImageDecoder::_fileExistsWithExtensions( const QString& fileName,
						const QStringList& extensionList) const
{
	QString baseFileName = fileName;
	int extStart = fileName.findRev('.') + 1;
	// We're interested in xxx.yyy, not .yyy
	if (extStart <= 1) return false;
	baseFileName.remove(extStart, baseFileName.length() - extStart);
	for ( QStringList::ConstIterator it = extensionList.begin();
	      it != extensionList.end(); ++it ) {
	    if (QFile::exists(baseFileName + *it)) return true;
	}
	return false;
}

bool RAWImageDecoder::_fileIsKnownWithExtensions( const Q3Dict<void>& files,
						 const QString& fileName,
						 const QStringList& extensionList) const
{
	QString baseFileName = fileName;
	int extStart = fileName.findRev('.') + 1;
	if (extStart <= 1) return false;
	baseFileName.remove(extStart, baseFileName.length() - extStart);
	for ( QStringList::ConstIterator it = extensionList.begin();
	      it != extensionList.end(); ++it ) {
	    if (files.find(baseFileName + *it) ) return true;
	}
	return false;
}

bool RAWImageDecoder::_fileEndsWithExtensions( const QString& fileName,
					      const QStringList& extensionList) const
{
	for ( QStringList::ConstIterator it = extensionList.begin();
	      it != extensionList.end(); ++it ) {
	    if( fileName.endsWith( *it, false ) ) return true;
	}
	return false;
}

bool RAWImageDecoder::_mightDecode( const QString& imageFile )
{
	if (Settings::SettingsData::instance()->dontReadRawFilesWithOtherMatchingFile() &&
	    _fileExistsWithExtensions(imageFile, _standardExtensions)) return false;
	if (_fileEndsWithExtensions(imageFile, _rawExtensions)) return true;
	return false;
}

bool RAWImageDecoder::_skipThisFile( const Q3Dict<void>& loadedFiles, const QString& imageFile )
{
	// We're not interested in thumbnail and other files.
	if (_fileEndsWithExtensions(imageFile, _ignoredExtensions)) return true;

	// If we *are* interested in raw files even when other equivalent
	// non-raw files are available, then we're interested in this file.
	if (! (Settings::SettingsData::instance()->dontReadRawFilesWithOtherMatchingFile())) return false;

	// If the file ends with something other than a known raw extension,
	// we're interested in it.
	if (! _fileEndsWithExtensions(imageFile, _rawExtensions)) return false;

	// At this point, the file ends with a known raw extension, and we're
	// not interested in raw files when other non-raw files are available.
	// So search for an existing file with one of the standard
	// extensions.
	//
	// This may not be the best way to do this, but it's using the
	// same algorithm as _mightDecode above.
	// -- Robert Krawitz rlk@alum.mit.edu 2007-07-22

	return _fileIsKnownWithExtensions(loadedFiles, imageFile, _standardExtensions);
}

}

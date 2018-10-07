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
#include "RawImageDecoder.h"
#include <config-kpa-kdcraw.h>
#include "Logging.h"

#include <DB/FileName.h>
#include "Settings/SettingsData.h"
#include <Utilities/FastJpeg.h>

#include <QFile>
#include <QImage>
#ifdef HAVE_KDCRAW
#  include <KDCRAW/KDcraw>
#  include <KDCRAW/RawFiles>
#  include <libkdcraw_version.h>
#endif

namespace ImageManager
{

bool RAWImageDecoder::_decode( QImage *img, const DB::FileName& imageFile, QSize* fullSize, int dim)
{
    /* width and height seem to be only hints, ignore */
    Q_UNUSED( dim );

#ifdef HAVE_KDCRAW
    QByteArray previewData;
    if ( !KDcrawIface::KDcraw::loadEmbeddedPreview( previewData, imageFile.absolute() ) )
        return false;

    // Faster than allowing loadRawPreview to do the decode itself
    if ( ! Utilities::loadJPEG(img, previewData, fullSize, dim ) )
        return false;

    // FIXME: The preview data for Canon's image is always returned in its non-rotated form by libkdcraw, ie. KPA should do the rotation.
    // FIXME: This will happen later on.
    if ( Settings::SettingsData::instance()->useRawThumbnail() &&
         ( ( dim > 0 && img->width() >= dim && img->height() >= dim ) ||
           ( img->width() >= Settings::SettingsData::instance()->useRawThumbnailSize().width() &&
             img->height() >= Settings::SettingsData::instance()->useRawThumbnailSize().height() ) ) )
        return true;

    KDcrawIface::DcrawInfoContainer metadata;
    if (!KDcrawIface::KDcraw::rawFileIdentify(metadata, imageFile.absolute()))
        return false;

    if ((img->width() < metadata.imageSize.width() * 0.8) ||
        (img->height() < metadata.imageSize.height() * 0.8)) {

        // let's try to get a better resolution
        KDcrawIface::KDcraw decoder;
        KDcrawIface::RawDecodingSettings rawDecodingSettings;

        if ( rawDecodingSettings.sixteenBitsImage ) {
            qCWarning(ImageManagerLog) << "16 bits per color channel is not supported yet";
            return false;
        } else {
            QByteArray imageData; /* 3 bytes for each pixel,  */
            int width, height, rgbmax;
            if ( !decoder.decodeRAWImage( imageFile.absolute(), rawDecodingSettings, imageData, width, height, rgbmax ) )
                return false;

            // Now the funny part, how to turn this fugly QByteArray into an QImage. Yay!
            *img = QImage(width, height, QImage::Format_RGB32);
            if (img->isNull())
                return false;

            uchar* data = img->bits();

            for ( int i = 0; i < imageData.size(); i += 3, data += 4 ) {
                data[0] = imageData[i + 2]; // blue
                data[1] = imageData[i + 1]; // green
                data[2] = imageData[i];     // red
                data[3] = 0xff;             // alpha
            }
        }
    }

    if ( fullSize )
        *fullSize = img->size();

    return true;
#else /* HAVE_KDCRAW */
    Q_UNUSED( img );
    Q_UNUSED( imageFile );
    Q_UNUSED( fullSize );
    return false;
#endif /* HAVE_KDCRAW */
}

void RAWImageDecoder::_initializeExtensionLists( QStringList& rawExtensions, QStringList& standardExtensions, QStringList& ignoredExtensions )
{
    static QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    static bool extensionListsInitialized = false;
    if ( ! extensionListsInitialized ) {
#ifdef HAVE_KDCRAW
        _rawExtensions = QString::fromLatin1( raw_file_extentions ).split( QChar::fromLatin1(' '), QString::SkipEmptyParts );
#endif /* HAVE_KDCRAW */
        for (QStringList::iterator it = _rawExtensions.begin(); it != _rawExtensions.end(); ++it)
            (*it).remove(QString::fromUtf8("*."));

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
                           << QString::fromLatin1("HTM")
                           << QString::fromLatin1("pp3") // RawTherapee Sidecar files
                           << QString::fromLatin1("PP3")
                           << QString::fromLatin1("xmp") // Other sidecars
                           << QString::fromLatin1("XMP")
                           << QString::fromLatin1("pto") // Hugin sidecars
                           << QString::fromLatin1("PTO");

        QChar dot( QChar::fromLatin1('.') );
        for ( QStringList::iterator it = _rawExtensions.begin(); it != _rawExtensions.end(); ++it )
            if ( !(*it).startsWith( dot) )
                *it = dot + *it;
        for ( QStringList::iterator it = _standardExtensions.begin(); it != _standardExtensions.end(); ++it )
            if ( !(*it).startsWith( dot) )
                *it = dot + *it;
        for ( QStringList::iterator it = _ignoredExtensions.begin(); it != _ignoredExtensions.end(); ++it )
            if ( !(*it).startsWith( dot) )
                *it = dot + *it;

        extensionListsInitialized = true;
    }

    rawExtensions = _rawExtensions;
    standardExtensions = _standardExtensions;
    ignoredExtensions = _ignoredExtensions;
}

bool RAWImageDecoder::_fileExistsWithExtensions( const DB::FileName& fileName,
                                                 const QStringList& extensionList) const
{
    QString baseFileName = fileName.absolute();
    int extStart = baseFileName.lastIndexOf(QChar::fromLatin1('.'));
    // We're interested in xxx.yyy, not .yyy
    if (extStart <= 1) return false;
    baseFileName.remove(extStart, baseFileName.length() - extStart);
    for ( QStringList::ConstIterator it = extensionList.begin();
          it != extensionList.end(); ++it ) {
        if (QFile::exists(baseFileName + *it)) return true;
    }
    return false;
}

bool RAWImageDecoder::_fileIsKnownWithExtensions( const DB::FileNameSet& files,
                                                  const DB::FileName& fileName,
                                                  const QStringList& extensionList) const
{
    QString baseFileName = fileName.absolute();
    int extStart = baseFileName.lastIndexOf(QChar::fromLatin1('.'));
    if (extStart <= 1) return false;
    baseFileName.remove(extStart, baseFileName.length() - extStart);
    for ( QStringList::ConstIterator it = extensionList.begin();
          it != extensionList.end(); ++it ) {
        if (files.contains(DB::FileName::fromAbsolutePath(baseFileName + *it)) )
            return true;
    }
    return false;
}

bool RAWImageDecoder::_fileEndsWithExtensions( const DB::FileName& fileName,
                                               const QStringList& extensionList)
{
    for ( QStringList::ConstIterator it = extensionList.begin();
          it != extensionList.end(); ++it ) {
        if( fileName.relative().endsWith( *it, Qt::CaseInsensitive ) ) return true;
    }
    return false;
}

bool RAWImageDecoder::_mightDecode( const DB::FileName& imageFile )
{
    QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    _initializeExtensionLists( _rawExtensions, _standardExtensions, _ignoredExtensions );

    if (Settings::SettingsData::instance()->skipRawIfOtherMatches() &&
            _fileExistsWithExtensions(imageFile, _standardExtensions)) return false;
    if (_fileEndsWithExtensions(imageFile, _rawExtensions)) return true;
    return false;
}

bool RAWImageDecoder::isRAW( const DB::FileName& imageFile )
{
    QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    _initializeExtensionLists( _rawExtensions, _standardExtensions, _ignoredExtensions );
    return _fileEndsWithExtensions(imageFile, _rawExtensions);
}

QStringList RAWImageDecoder::rawExtensions()
{
    QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    _initializeExtensionLists( _rawExtensions, _standardExtensions, _ignoredExtensions );
    return _rawExtensions;
}

bool RAWImageDecoder::_skipThisFile( const DB::FileNameSet& loadedFiles, const DB::FileName& imageFile ) const
{
    QStringList _rawExtensions, _standardExtensions, _ignoredExtensions;
    _initializeExtensionLists( _rawExtensions, _standardExtensions, _ignoredExtensions );

    // We're not interested in thumbnail and other files.
    if (_fileEndsWithExtensions(imageFile, _ignoredExtensions)) return true;

    // If we *are* interested in raw files even when other equivalent
    // non-raw files are available, then we're interested in this file.
    if (! (Settings::SettingsData::instance()->skipRawIfOtherMatches())) return false;

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
// vi:expandtab:tabstop=4 shiftwidth=4:

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
#include "Exif/Info.h"
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>
#include "Utilities/Set.h"
#include "Settings/SettingsData.h"
#include "Info.h"
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include <qfileinfo.h>
#include <qfile.h>
#include "Utilities/Util.h"

using namespace Exif;

Info* Info::_instance = 0;

QMap<QString, QStringList> Info::info( const QString& fileName, const StringSet& wantedKeys, bool returnFullExifName, const QString& charset )
{
    QMap<QString, QStringList> result;

    try {
        Metadata data = metadata( fileName );

        if ( !data.exif.empty()) {
            Exiv2::ExifData::const_iterator end = data.exif.end();

            for (Exiv2::ExifData::const_iterator i = data.exif.begin(); i != end; ++i) {
                QString key = QString::fromLocal8Bit(i->key().c_str());
                _keys.insert( key );

                if ( wantedKeys.contains( key ) ) {
                    QString text = key;
                    if ( !returnFullExifName )
                        text = QStringList::split( QString::fromLatin1("."), key ).last();

                    std::string str;
                    std::ostringstream stream;
                    stream << *i;
                    str = stream.str();
                    result.insert( text, QString::fromLocal8Bit(str.c_str()) );
                }
            }
        }
        
        if ( !data.iptc.empty()) {
            Exiv2::IptcData::const_iterator end = data.iptc.end();

            for (Exiv2::IptcData::const_iterator i = data.iptc.begin(); i != end; ++i) {
                QString key = QString::fromLatin1(i->key().c_str());
                _keys.insert( key );

                if ( wantedKeys.contains( key ) ) {
                    QString text = key;
                    if ( !returnFullExifName )
                        text = QStringList::split( QString::fromLatin1("."), key ).last();

                    std::ostringstream stream;
                    stream << *i;
                    QString str( Utilities::cStringWithEncoding( stream.str().c_str(), charset ) );
                    if ( result.contains( text ) )
                        result[ text ] += str;
                    else
                        result.insert( text, str );
                }
            }
        }

    }
    catch ( ... ) {
    }

    return result;
}

Info* Info::instance()
{
    if ( !_instance )
        _instance = new Info;
    return _instance;
}

StringSet Info::availableKeys()
{
    return _keys;
}

QMap<QString, QStringList> Info::infoForViewer( const QString& fileName, bool returnFullExifName )
{
    return info( fileName, ::Settings::SettingsData::instance()->exifForViewer(),
    		 returnFullExifName, ::Utilities::humanReadableCharsetList()[ ::Settings::SettingsData::instance()->iptcCharset() ] );
}

QMap<QString, QStringList> Info::infoForDialog( const QString& fileName, const QString& charset )
{
    return info( fileName, ::Settings::SettingsData::instance()->exifForDialog(), true, charset);
}

StringSet Info::standardKeys()
{
    StringSet res;

    // Standard EXIF
    res.insert( QString::fromLatin1( "Exif.Image.NewSubfileType" ) );
    res.insert( QString::fromLatin1( "Exif.Image.ImageWidth" ) );
    res.insert( QString::fromLatin1( "Exif.Image.ImageLength" ) );
    res.insert( QString::fromLatin1( "Exif.Image.BitsPerSample" ) );
    res.insert( QString::fromLatin1( "Exif.Image.Compression" ) );
    res.insert( QString::fromLatin1( "Exif.Image.PhotometricInterpretation" ) );
    res.insert( QString::fromLatin1( "Exif.Image.FillOrder" ) );
    res.insert( QString::fromLatin1( "Exif.Image.DocumentName" ) );
    res.insert( QString::fromLatin1( "Exif.Image.ImageDescription" ) );
    res.insert( QString::fromLatin1( "Exif.Image.Make" ) );
    res.insert( QString::fromLatin1( "Exif.Image.Model" ) );
    res.insert( QString::fromLatin1( "Exif.Image.StripOffsets" ) );
    res.insert( QString::fromLatin1( "Exif.Image.Orientation" ) );
    res.insert( QString::fromLatin1( "Exif.Image.SamplesPerPixel" ) );
    res.insert( QString::fromLatin1( "Exif.Image.RowsPerStrip" ) );
    res.insert( QString::fromLatin1( "Exif.Image.StripByteCounts" ) );
    res.insert( QString::fromLatin1( "Exif.Image.XResolution" ) );
    res.insert( QString::fromLatin1( "Exif.Image.YResolution" ) );
    res.insert( QString::fromLatin1( "Exif.Image.PlanarConfiguration" ) );
    res.insert( QString::fromLatin1( "Exif.Image.ResolutionUnit" ) );
    res.insert( QString::fromLatin1( "Exif.Image.TransferFunction" ) );
    res.insert( QString::fromLatin1( "Exif.Image.Software" ) );
    res.insert( QString::fromLatin1( "Exif.Image.DateTime" ) );
    res.insert( QString::fromLatin1( "Exif.Image.Artist" ) );
    res.insert( QString::fromLatin1( "Exif.Image.WhitePoint" ) );
    res.insert( QString::fromLatin1( "Exif.Image.PrimaryChromaticities" ) );
    res.insert( QString::fromLatin1( "Exif.Image.SubIFDs" ) );
    res.insert( QString::fromLatin1( "Exif.Image.TransferRange" ) );
    res.insert( QString::fromLatin1( "Exif.Image.JPEGProc" ) );
    res.insert( QString::fromLatin1( "Exif.Image.JPEGInterchangeFormat" ) );
    res.insert( QString::fromLatin1( "Exif.Image.JPEGInterchangeFormatLength" ) );
    res.insert( QString::fromLatin1( "Exif.Image.YCbCrCoefficients" ) );
    res.insert( QString::fromLatin1( "Exif.Image.YCbCrSubSampling" ) );
    res.insert( QString::fromLatin1( "Exif.Image.YCbCrPositioning" ) );
    res.insert( QString::fromLatin1( "Exif.Image.ReferenceBlackWhite" ) );
    res.insert( QString::fromLatin1( "Exif.Image.XMLPacket" ) );
    res.insert( QString::fromLatin1( "Exif.Image.CFARepeatPatternDim" ) );
    res.insert( QString::fromLatin1( "Exif.Image.CFAPattern" ) );
    res.insert( QString::fromLatin1( "Exif.Image.BatteryLevel" ) );
    res.insert( QString::fromLatin1( "Exif.Image.IPTCNAA" ) );
    res.insert( QString::fromLatin1( "Exif.Image.Copyright" ) );
    res.insert( QString::fromLatin1( "Exif.Image.ImageResources" ) );
    res.insert( QString::fromLatin1( "Exif.Image.ExifTag" ) );
    res.insert( QString::fromLatin1( "Exif.Image.InterColorProfile" ) );
    res.insert( QString::fromLatin1( "Exif.Image.GPSTag" ) );
    res.insert( QString::fromLatin1( "Exif.Image.TIFFEPStandardID" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ExposureTime" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FNumber" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ExposureProgram" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SpectralSensitivity" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ISOSpeedRatings" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.OECF" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ExifVersion" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.DateTimeOriginal" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.DateTimeDigitized" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ComponentsConfiguration" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.CompressedBitsPerPixel" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ShutterSpeedValue" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ApertureValue" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.BrightnessValue" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ExposureBiasValue" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.MaxApertureValue" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SubjectDistance" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.MeteringMode" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.LightSource" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.Flash" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FocalLength" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SubjectArea" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.MakerNote" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.UserComment" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SubSecTime" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SubSecTimeOriginal" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SubSecTimeDigitized" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FlashpixVersion" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ColorSpace" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.PixelXDimension" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.PixelYDimension" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.RelatedSoundFile" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.InteroperabilityTag" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FlashEnergy" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SpatialFrequencyResponse" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FocalPlaneXResolution" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FocalPlaneYResolution" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FocalPlaneResolutionUnit" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SubjectLocation" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ExposureIndex" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SensingMethod" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FileSource" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SceneType" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.CFAPattern" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.CustomRendered" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ExposureMode" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.WhiteBalance" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.DigitalZoomRatio" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.FocalLengthIn35mmFilm" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SceneCaptureType" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.GainControl" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.Contrast" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.Saturation" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.Sharpness" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.DeviceSettingDescription" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.SubjectDistanceRange" ) );
    res.insert( QString::fromLatin1( "Exif.Photo.ImageUniqueID" ) );
    res.insert( QString::fromLatin1( "Exif.Iop.InteroperabilityIndex" ) );
    res.insert( QString::fromLatin1( "Exif.Iop.InteroperabilityVersion" ) );
    res.insert( QString::fromLatin1( "Exif.Iop.RelatedImageFileFormat" ) );
    res.insert( QString::fromLatin1( "Exif.Iop.RelatedImageWidth" ) );
    res.insert( QString::fromLatin1( "Exif.Iop.RelatedImageLength" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSVersionID" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSLatitudeRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSLatitude" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSLongitudeRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSLongitude" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSAltitudeRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSAltitude" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSTimeStamp" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSSatellites" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSStatus" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSMeasureMode" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDOP" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSSpeedRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSSpeed" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSTrackRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSTrack" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSImgDirectionRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSImgDirection" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSMapDatum" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDestLatitudeRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDestLatitude" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDestLongitudeRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDestLongitude" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDestBearingRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDestBearing" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDestDistanceRef" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDestDistance" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSProcessingMethod" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSAreaInformation" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDateStamp" ) );
    res.insert( QString::fromLatin1( "Exif.GPSInfo.GPSDifferential" ) );

    // Panasonic Makernote
    res.insert( QString::fromLatin1( "Exif.Panasonic.Quality" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.FirmwareVersion" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.WhiteBalance" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.0x0004" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.FocusMode" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.SpotMode" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.ImageStabilizer" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.Macro" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.ShootingMode" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.Audio" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.DataDump" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.0x0022" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.WhiteBalanceBias" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.FlashBias" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.SerialNumber" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.0x0026" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.0x0027" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.ColorEffect" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.0x0029" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.BurstMode" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.SequenceNumber" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.Contrast" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.NoiseReduction" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.SelfTimer" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.0x002f" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.Rotation" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.0x0031" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.ColorMode" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.TravelDay" ) );
    res.insert( QString::fromLatin1( "Exif.Minolta.PrintIM" ) );
    res.insert( QString::fromLatin1( "Exif.Panasonic.0x4449" ) );

    // Canon Makernote
    res.insert( QString::fromLatin1( "Exif.Canon.0x0000" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.CameraSettings" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.0x0002" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.0x0003" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.ShotInfo" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.Panorama" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.ImageType" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.FirmwareVersion" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.ImageNumber" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.OwnerName" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.SerialNumber" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.0x000d" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.CustomFunctions" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.PictureInfo" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.WhiteBalanceTable" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.0x00b5" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.0x00c0" ) );
    res.insert( QString::fromLatin1( "Exif.Canon.0x00c1" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.Macro" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.Selftimer" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.Quality" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.FlashMode" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.DriveMode" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0006" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.FocusMode" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0008" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0009" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.ImageSize" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.EasyMode" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.DigitalZoom" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.Contrast" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.Saturation" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.Sharpness" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.ISOSpeed" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.MeteringMode" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.FocusType" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.AFPoint" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.ExposureProgram" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0015" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0016" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.Lens" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0018" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0019" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x001a" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x001b" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.FlashActivity" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.FlashDetails" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x001e" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x001f" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.FocusContinuous" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.AESetting" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.ImageStabilization" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.DisplayAperture" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.ZoomSourceWidth" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.ZoomTargetWidth" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0026" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0027" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.PhotoEffect" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.0x0029" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCs.ColorTone" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0001" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.ISOSpeed" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0003" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.TargetAperture" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.TargetShutterSpeed" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0006" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.WhiteBalance" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0008" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.Sequence" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x000a" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x000b" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x000c" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x000d" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.AFPointUsed" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.FlashBias" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0010" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0011" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0012" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.SubjectDistance" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0014" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.ApertureValue" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.ShutterSpeedValue" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0017" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0018" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x0019" ) );
    res.insert( QString::fromLatin1( "Exif.CanonSi.0x001a" ) );
    res.insert( QString::fromLatin1( "Exif.CanonPa.PanoramaFrame" ) );
    res.insert( QString::fromLatin1( "Exif.CanonPa.PanoramaDirection" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.NoiseReduction" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.ShutterAeLock" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.MirrorLockup" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.ExposureLevelIncrements" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.AFAssist" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.FlashSyncSpeedAv" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.AEBSequence" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.ShutterCurtainSync" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.LensAFStopButton" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.FillFlashAutoReduction" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.MenuButtonReturn" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.SetButtonFunction" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.SensorCleaning" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.SuperimposedDisplay" ) );
    res.insert( QString::fromLatin1( "Exif.CanonCf.ShutterReleaseNoCFCard" ) );
    res.insert( QString::fromLatin1( "Exif.CanonPi.ImageWidth" ) );
    res.insert( QString::fromLatin1( "Exif.CanonPi.ImageHeight" ) );
    res.insert( QString::fromLatin1( "Exif.CanonPi.ImageWidthAsShot" ) );
    res.insert( QString::fromLatin1( "Exif.CanonPi.ImageHeightAsShot" ) );
    res.insert( QString::fromLatin1( "Exif.CanonPi.AFPointsUsed" ) );
    res.insert( QString::fromLatin1( "Exif.CanonPi.AFPointsUsed20D" ) );

    // IPTC
    res.insert( QString::fromLatin1( "Iptc.Envelope.ModelVersion" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.Destination" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.FileFormat" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.FileVersion" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.ServiceId" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.EnvelopeNumber" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.ProductId" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.EnvelopePriority" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.DateSent" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.TimeSent" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.CharacterSet" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.UNO" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.ARMId" ) );
    res.insert( QString::fromLatin1( "Iptc.Envelope.ARMVersion" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.RecordVersion" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ObjectType" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ObjectAttribute" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ObjectName" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.EditStatus" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.EditorialUpdate" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Urgency" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Subject" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Category" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.SuppCategory" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.FixtureId" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Keywords" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.LocationCode" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.LocationName" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ReleaseDate" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ReleaseTime" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ExpirationDate" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ExpirationTime" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.SpecialInstructions" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ActionAdvised" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ReferenceService" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ReferenceDate" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ReferenceNumber" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.DateCreated" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.TimeCreated" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.DigitizationDate" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.DigitizationTime" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Program" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ProgramVersion" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ObjectCycle" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Byline" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.BylineTitle" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.City" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.SubLocation" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ProvinceState" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.CountryCode" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.CountryName" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.TransmissionReference" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Headline" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Credit" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Source" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Copyright" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Contact" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Caption" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Writer" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.RasterizedCaption" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ImageType" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.ImageOrientation" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Language" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.AudioType" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.AudioRate" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.AudioResolution" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.AudioDuration" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.AudioOutcue" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.PreviewFormat" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.PreviewVersion" ) );
    res.insert( QString::fromLatin1( "Iptc.Application2.Preview" ) );

    return res;
}

Info::Info()
{
    _keys = standardKeys();
}

void Exif::Info::writeInfoToFile( const QString& srcName, const QString& destName )
{
    // Load Exif from source image
    Exiv2::Image::AutoPtr image =
        Exiv2::ImageFactory::open( QFile::encodeName(srcName).data() );
    image->readMetadata();
    Exiv2::ExifData data = image->exifData();
    Exiv2::IptcData iData = image->iptcData();
    std::string comment = image->comment();

    // Modify Exif information from database.
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( srcName );
    data["Exif.Image.ImageDescription"] = info->description().local8Bit().data();

    image = Exiv2::ImageFactory::open( QFile::encodeName(destName).data() );
    image->setExifData( data );
    image->setIptcData( iData );
    image->setComment( comment );
    image->writeMetadata();
}

/**
 * Some Canon cameras stores EXIF info in files ending in .thm, so we need to use those files for fetching EXIF info
 * if they exists.
 */
QString Exif::Info::exifInfoFile( const QString& fileName )
{
    QString dirName = QFileInfo( fileName ).dirPath();
    QString baseName = QFileInfo( fileName ).baseName();
    QString name = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".thm" );
    if ( QFileInfo(name).exists() )
        return name;

    name = dirName + QString::fromLatin1("/") + baseName + QString::fromLatin1( ".THM" );
    if ( QFileInfo(name).exists() )
        return name;

    return fileName;
}

Exif::Metadata Exif::Info::metadata( const QString& fileName )
{
    try {
        Exif::Metadata result;
        Exiv2::Image::AutoPtr image =
            Exiv2::ImageFactory::open(QFile::encodeName(fileName).data());
        Q_ASSERT(image.get() != 0);
        image->readMetadata();
        result.exif = image->exifData();
        result.iptc = image->iptcData();
        result.comment = image->comment();
        return result;
    }
    catch ( ... ) {
    }
    return Exif::Metadata();
}

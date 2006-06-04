#include "Exif/Info.h"
#include "exiv2/image.hpp"
#include "exiv2/exif.hpp"
#include "Utilities/Set.h"
#include "Settings/SettingsData.h"
#include <qsqlquery.h>
#include <iostream>
#include <sstream>
#include "Info.h"
#include "DB/ImageInfo.h"
#include "DB/ImageDB.h"
#include <qfileinfo.h>

using namespace Exif;

Info* Info::_instance = 0;

QMap<QString, QString> Info::info( const QString& fileName, Set<QString> wantedKeys, bool returnFullExifName )
{
    QMap<QString, QString> result;

    try {
        Exiv2::ExifData data = exifData( fileName );
        if (data.empty()) {
            return result;
        }

        Exiv2::ExifData::const_iterator end = data.end();

        for (Exiv2::ExifData::const_iterator i = data.begin(); i != end; ++i) {
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

Set<QString> Info::availableKeys()
{
    return _keys;
}

QMap<QString, QString> Info::infoForViewer( const QString& fileName )
{
    return info( fileName, Settings::SettingsData::instance()->exifForViewer(), false );
}

QMap<QString, QString> Info::infoForDialog( const QString& fileName )
{
    return info( fileName, Settings::SettingsData::instance()->exifForDialog(), true);
}

Set<QString> Info::standardKeys()
{
    Set<QString> res;
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

    return res;
}

Info::Info()
{
    _keys = standardKeys();
}

void Exif::Info::writeInfoToFile( const QString& srcName, const QString& destName )
{
    // Load Exif from source image
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(srcName.local8Bit().data());
    image->readMetadata();
    Exiv2::ExifData data = image->exifData();

    // Modify Exif information from database.
    DB::ImageInfoPtr info = DB::ImageDB::instance()->info( srcName );
    data["Exif.Image.ImageDescription"] = info->description().local8Bit().data();

    image = Exiv2::ImageFactory::open( destName.local8Bit().data() );
    image->setExifData(data);
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

Exiv2::ExifData Exif::Info::exifData( const QString& fileName )
{
    try {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fileName.local8Bit().data());
        Q_ASSERT(image.get() != 0);
        image->readMetadata();

        return image->exifData();
    }
    catch ( ... ) {
    }
    return Exiv2::ExifData();
}


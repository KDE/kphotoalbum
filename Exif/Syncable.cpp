/* Copyright (C) 2007 Jan Kundrát <jkt@gentoo.org>

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
#include <klocale.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qmap.h>
#include <qwhatsthis.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include "Exif/Syncable.h"

/**
 * Helper function that fills the translation tables passed as references with
 * stuff like Exif/IPTC tag names, kind of field, displayable name,...
 * */
void Exif::Syncable::fillTranslationTables( QMap<Exif::Syncable::Kind,QString>& _fieldName,
        QMap<Exif::Syncable::Kind,QString>& _visibleName, QMap<Exif::Syncable::Kind,Header>& _header)
{
#define II(X,HEADER,FIELD,VISIBLE) \
     _header[X] = HEADER; \
     _fieldName[X] = #FIELD; \
     _visibleName[X] = i18n(VISIBLE);
#define I(X,HEADER,FIELD) \
     _header[X] = HEADER; \
     _fieldName[X] = #FIELD; \
     _visibleName[X] = QString::fromAscii(#FIELD);


    II(STOP, NONE, STOP, "-- stop --");
    II(JPEG_COMMENT, JPEG, JPEG.Comment, "JPEG Comment");
    II(EXIF_ORIENTATION, EXIF, Exif.Image.Orientation, "EXIF Image Orientation");
    II(JPEG_ORIENTATION, JPEG, JPEGImage.Orientation, "Physical JPEG Rotation");
    II(EXIF_DESCRIPTION, EXIF, Exif.Image.ImageDescription, "EXIF Image Description");
    II(EXIF_USER_COMMENT, EXIF, Exif.Photo.UserComment, "EXIF User Comment");

    I(EXIF_XPTITLE, EXIF, Exif.Image.XPTitle);
    I(EXIF_XPCOMMENT, EXIF, Exif.Image.XPComment);
    I(EXIF_XPKEYWORDS, EXIF, Exif.Image.XPKeywords);
    I(EXIF_XPSUBJECT, EXIF, Exif.Image.XPSubject);

    II(IPTC_HEADLINE, IPTC, Iptc.Application2.Headline, "IPTC Headline");
    II(IPTC_CAPTION, IPTC, Iptc.Application2.Caption, "IPTC Caption");
    I(IPTC_OBJECT_NAME, IPTC, Iptc.Application2.ObjectName);
    II(IPTC_SUBJECT, IPTC, Iptc.Application2.Subject, "IPTC Subject");

    II(IPTC_SUPP_CAT, IPTC, Iptc.Application2.SuppCategory, "IPTC Supplemental Categories");
    II(IPTC_KEYWORDS, IPTC, Iptc.Application2.Keywords, "IPTC Keywords");

    I(IPTC_LOCATION_CODE, IPTC, Iptc.Application2.LocationCode);
    I(IPTC_LOCATION_NAME, IPTC, Iptc.Application2.LocationName);
    I(IPTC_CITY, IPTC, Iptc.Application2.City);
    I(IPTC_SUB_LOCATION, IPTC, Iptc.Application2.SubLocation);
    I(IPTC_PROVINCE_STATE, IPTC, Iptc.Application2.ProvinceState);
    I(IPTC_COUNTRY_CODE, IPTC, Iptc.Application2.CountryCode);
    I(IPTC_COUNTRY_NAME, IPTC, Iptc.Application2.CountryName);

    II(FILE_CTIME, FILE, File.CTime, "File creation time");
    II(FILE_MTIME, FILE, File.MTime, "File last modification time");
    II(EXIF_DATETIME, EXIF, Exif.Image.DateTime, "EXIF Date");
    I(EXIF_DATETIME_ORIGINAL, EXIF, Exif.Photo.DateTimeOriginal);
    I(EXIF_DATETIME_DIGITIZED, EXIF, Exif.Photo.DateTimeDigitized);

    II(FILE_NAME, FILE, File.Name, "File name");

#undef I
#undef II
}


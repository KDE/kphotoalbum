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
#ifndef EXIFSYNCABLE_H
#define EXIFSYNCABLE_H

template <class Key, class T> class QMap;

namespace Exif {

namespace Syncable {

enum Header { NONE, FILE, JPEG, EXIF, IPTC };

enum Kind { 
    // delimiter for the "don't process more records" position
    STOP,
    // JPEG header
    JPEG_COMMENT,
    // image orientation
    EXIF_ORIENTATION, JPEG_ORIENTATION,
    // standard EXIF fields for description/label
    EXIF_DESCRIPTION, EXIF_USER_COMMENT,
    // stuff from Windows XP
    EXIF_XPTITLE, EXIF_XPCOMMENT, EXIF_XPKEYWORDS, EXIF_XPSUBJECT,
    // standard IPTC label/description
    IPTC_HEADLINE, IPTC_CAPTION, IPTC_OBJECT_NAME, IPTC_SUBJECT,
    // IPTC categories
    IPTC_SUPP_CAT, IPTC_KEYWORDS,
    // geographic stuff
    IPTC_LOCATION_CODE, IPTC_LOCATION_NAME, IPTC_CITY,
    IPTC_SUB_LOCATION, IPTC_PROVINCE_STATE, IPTC_COUNTRY_CODE, IPTC_COUNTRY_NAME,
    // file modification time
    FILE_CTIME, FILE_MTIME, EXIF_DATETIME, EXIF_DATETIME_ORIGINAL, EXIF_DATETIME_DIGITIZED,
    // file name
    FILE_NAME
};

enum SuperGroupHandling {
    // Treat all levels separately. Europe/Prague is transformed to two tags,
    // "Europe" and "Prague"
    Independent,
    // Include only the most-specific one -> "Prague"
    MostSpecific,
    // Create new string value by joining by slash -> "Europe/Prague"
    MergeBySlash
};

enum MultiValueHandling {
    // repeat the requested field multiple times
    Repeat,
    // separate all values with commas
    SeparateComma,
    // separate by semicolon
    SeparateSemicolon
};

void fillTranslationTables( QMap<Kind,QString>& _fieldName, QMap<Kind,QString>& _visibleName, QMap<Kind,Header>& _header);

}

}

#endif /* EXIFSYNCABLE_H */


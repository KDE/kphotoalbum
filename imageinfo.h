/* Copyright (C) 2003-2005 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qvaluelist.h>
#include <qpixmap.h>
#include <qdom.h>
#include <qobject.h>
#include <qdeepcopy.h>
#include "imagedate.h"
#include "drawlist.h"
#include <qimage.h>
#include "imagedaterange.h"

#define EXIFMODE_TIME          0x01
#define EXIFMODE_DATE          0x02
#define EXIFMODE_ORIENTATION   0x04
#define EXIFMODE_DESCRIPTION   0x08
#define EXIFMODE_FORCE         0x10
#define EXIFMODE_FORCE_TIME    0x20
#define EXIFMODE_FORCE_DATE    0x40
#define EXIFMODE_INIT ( EXIFMODE_TIME | EXIFMODE_DATE | EXIFMODE_ORIENTATION | EXIFMODE_DESCRIPTION | EXIFMODE_FORCE_TIME | EXIFMODE_FORCE_DATE )

class ImageInfo {

public:

    ImageInfo();
    ImageInfo( const QString& fileName );
    ImageInfo( const QString& fileName, QDomElement elm );
    void setVisible( bool b );
    bool visible() const;

    QString fileName( bool relative = false ) const;
    void setFileName( const QString& relativeFileName );

    void setLabel( const QString& );
    QString label() const;

    void setDescription( const QString& );
    QString description() const;

    void setStartDate( const ImageDate& );
    void setEndDate( const ImageDate& );
    ImageDate& startDate();
    ImageDate& endDate();
    ImageDateRange dateRange() const;

    void readExif(const QString& fullPath, int mode);

    void rotate( int degrees );
    int angle() const;
    void setAngle( int angle );

    void setOption( const QString& key,  const QStringList& value );
    void addOption( const QString& key,  const QStringList& value );
    void removeOption( const QString& key, const QString& value );
    bool hasOption( const QString& key,  const QString& value );
    QStringList availableOptionGroups() const;
    QStringList optionValue( const QString& key ) const;
    void renameOption( const QString& key, const QString& oldValue, const QString& newValue );
    void renameOptionGroup( const QString& oldName, const QString& newName );

    QDomElement save( QDomDocument doc );
    bool operator!=( const ImageInfo& other );
    bool operator==( const ImageInfo& other );

    DrawList drawList() const;
    void setDrawList( const DrawList& );

    bool imageOnDisk() const;
    void setImageOnDisk( bool b );
    QString MD5Sum() const { return _md5sum; }
    void setMD5Sum( const QString& sum ) { _md5sum = sum; }

    void setLocked( bool );
    bool isLocked() const;

    bool isNull() const { return _null; }
    QSize size() const;
    void setSize( const QSize& size );

    // Used during searches
    void clearMatched() const;
    void setMatched( const QString& category, const QString& value ) const;
    bool allMatched( const QString& category );

protected:
    bool loadJPEG(QImage* image, const QString& fileName ) const;
    bool isJPEG( const QString& fileName ) const;

private:
    friend class MyImageInfo;
    QString _fileName;
    QString _label;
    QString _description;
    ImageDate _startDate, _endDate;
    QMap<QString, QStringList> _options;
    int _angle;
    bool _visible;
    DrawList _drawList;
    enum OnDisk { YesOnDisk, NoNotOnDisk, Unchecked };
    mutable OnDisk _imageOnDisk;
    QString _md5sum;
    bool _null;
    QSize _size;

    // Cache information
    bool _locked;

    // Used during searching to make it possible to search for Jesper & None
    mutable QMap<QString,QStringList> _matched;

public:
    // used for checking if any images are without image attribute from the database.
    static bool _anyImageWithEmptySize;
};

#endif /* IMAGEINFO_H */


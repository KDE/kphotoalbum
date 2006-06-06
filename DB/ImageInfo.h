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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
#include "ImageDate.h"
#include "Viewer/DrawList.h"
#include <qimage.h>
#include <ksharedptr.h>
#include "DB/CategoryCollection.h"
#include "Utilities/Set.h"

#define EXIFMODE_DATE          0x01
#define EXIFMODE_ORIENTATION   0x02
#define EXIFMODE_DESCRIPTION   0x04
#define EXIFMODE_FORCE         0x08
#define EXIFMODE_FORCE_DATE    0x10
#define EXIFMODE_DATABASE_UPDATE 0x20
#define EXIFMODE_INIT ( EXIFMODE_DATE | EXIFMODE_ORIENTATION | EXIFMODE_DESCRIPTION | EXIFMODE_FORCE_DATE | EXIFMODE_DATABASE_UPDATE )

namespace Plugins
{
     class ImageInfo;
}

namespace DB
{

enum MediaType { Image = 0x01, Movie = 0x10 };

class ImageInfo :public KShared {

public:

    ImageInfo();
    ImageInfo( const QString& fileName, MediaType type = Image );
    ImageInfo( const QString& fileName,
               const QString& label,
               const QString& description,
               const ImageDate& date,
               int angle,
               const QString& md5sum,
               const QSize& size,
               MediaType type);

    QString fileName( bool relative = false ) const;
    void setFileName( const QString& relativeFileName );

    void setLabel( const QString& );
    QString label() const;

    void setDescription( const QString& );
    QString description() const;

    void setDate( const ImageDate& );
    ImageDate date() const;
    ImageDate& date();
    void readExif(const QString& fullPath, int mode);

    void rotate( int degrees );
    int angle() const;
    void setAngle( int angle );

    void setOption( const QString& key,  const QStringList& value );
    void addOption( const QString& key,  const QStringList& value );
    void removeOption( const QString& key, const QStringList& value );
    void removeOption( const QString& key, const QString& value );
    bool hasOption( const QString& key,  const QString& value );
    QStringList availableCategories() const;
    QStringList itemsOfCategory( const QString& category ) const;
    void renameItem( const QString& key, const QString& oldValue, const QString& newValue );
    void renameCategory( const QString& oldName, const QString& newName );

    bool operator!=( const ImageInfo& other );
    bool operator==( const ImageInfo& other );
    virtual ImageInfo& operator=( const ImageInfo& other );

    Viewer::DrawList drawList() const;
    void setDrawList( const Viewer::DrawList& );
    void addDrawing( const QDomElement& );

    bool imageOnDisk() const;
    static bool imageOnDisk( const QString& fileName );

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

    MediaType mediaType() const;

protected:
    bool loadJPEG(QImage* image, const QString& fileName ) const;
    bool isJPEG( const QString& fileName ) const;
    void setAbsoluteFileName();

private:
    friend class Plugins::ImageInfo;
    QString _relativeFileName;
    QString _absoluteFileName;
    QString _label;
    QString _description;
    ImageDate _date;
    QMap<QString, QStringList> _options;
    int _angle;
    Viewer::DrawList _drawList;
    enum OnDisk { YesOnDisk, NoNotOnDisk, Unchecked };
    mutable OnDisk _imageOnDisk;
    QString _md5sum;
    bool _null;
    QSize _size;
    MediaType _type;

    // Cache information
    bool _locked;

    // Used during searching to make it possible to search for Jesper & None
    mutable QMap<QString, Set<QString> > _matched;
};

}

#endif /* IMAGEINFO_H */


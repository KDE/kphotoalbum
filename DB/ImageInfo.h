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

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qdom.h>
#include "ImageDate.h"
#include "Viewer/DrawList.h"
#include <qimage.h>
#include "DB/CategoryCollection.h"
#include "Utilities/Set.h"
#include "MD5.h"

#define EXIFMODE_DATE          0x01
#define EXIFMODE_ORIENTATION   0x02
#define EXIFMODE_DESCRIPTION   0x04
#define EXIFMODE_LABEL         0x08
#define EXIFMODE_CATEGORIES    0x10
#define EXIFMODE_DATABASE_UPDATE 0x20
#define EXIFMODE_INIT ( EXIFMODE_DATE | EXIFMODE_ORIENTATION | EXIFMODE_DESCRIPTION | EXIFMODE_LABEL | EXIFMODE_CATEGORIES | EXIFMODE_DATABASE_UPDATE )
// EXIFMODE_INIT is all of that modulo categories to prevent category cluttering

namespace Plugins
{
     class ImageInfo;
}

namespace DB
{
using Utilities::StringSet;

class MemberMap;

enum MediaType { Image = 0x01, Video = 0x02 };
const MediaType anyMediaType = MediaType(Image | Video);

class ImageInfo :public KShared {

public:

    ImageInfo();
    ImageInfo( const QString& fileName, MediaType type = Image );
    ImageInfo( const QString& fileName,
               const QString& label,
               const QString& description,
               const ImageDate& date,
               int angle,
               const MD5& md5sum,
               const QSize& size,
               MediaType type);
    virtual ~ImageInfo() { saveChanges(); }

    QString fileName( bool relative = false ) const;
    void setFileName( const QString& relativeFileName );

    void setLabel( const QString& );
    QString label() const;

    void setDescription( const QString& );
    QString description() const;

    void setDate( const ImageDate& );
    ImageDate date() const;
    ImageDate& date();
    void readExif(const QString& fullPath, const int mode);
    void writeMetadata(const QString& fullPath, const int mode);

    void rotate( int degrees );
    int angle() const;
    void setAngle( int angle );

    void setCategoryInfo( const QString& key,  const StringSet& value );
    void addCategoryInfo( const QString& category, const StringSet& values );
    void addCategoryInfo( const QString& category, const QString& value );
    void removeCategoryInfo( const QString& category, const StringSet& values );
    void removeCategoryInfo( const QString& category, const QString& value );

    bool hasCategoryInfo( const QString& key,  const QString& value );
    QStringList availableCategories() const;
    StringSet itemsOfCategory( const QString& category ) const;
    void renameItem( const QString& key, const QString& oldValue, const QString& newValue );
    void renameCategory( const QString& oldName, const QString& newName );

    bool operator!=( const ImageInfo& other );
    bool operator==( const ImageInfo& other );
    virtual ImageInfo& operator=( const ImageInfo& other );

    Viewer::DrawList drawList() const;
    void setDrawList( const Viewer::DrawList& );
    void addDrawing( const QDomElement& );

    static bool imageOnDisk( const QString& fileName );

    const MD5& MD5Sum() const { return _md5sum; }
    void setMD5Sum( const MD5& sum ) { if (sum != _md5sum) _dirty = true; _md5sum = sum; saveChangesIfNotDelayed(); }

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
    void setMediaType( MediaType type ) { if (type != _type) _dirty = true; _type = type; saveChangesIfNotDelayed(); }

    void createFolderCategoryItem( DB::Category*, DB::MemberMap& memberMap );

    void delaySavingChanges(bool b=true);

protected:
    /** Save changes to database.
     *
     * Back-ends, which need changes to be instantly in database,
     * should override this.
     */
    virtual void saveChanges() {}

    void saveChangesIfNotDelayed() { if (!_delaySaving) saveChanges(); }

    void setAbsoluteFileName();
    void setIsNull(bool b) { _null = b; }
    bool isDirty() const { return _dirty; }
    void setIsDirty(bool b)  { _dirty = b; }

private:
    friend class Plugins::ImageInfo;
    QString _relativeFileName;
    QString _absoluteFileName;
    QString _label;
    QString _description;
    ImageDate _date;
    QMap<QString, StringSet> _categoryInfomation;
    int _angle;
    Viewer::DrawList _drawList;
    enum OnDisk { YesOnDisk, NoNotOnDisk, Unchecked };
    mutable OnDisk _imageOnDisk;
    MD5 _md5sum;
    bool _null;
    QSize _size;
    MediaType _type;

    // Cache information
    bool _locked;

    // Will be set to true after every change
    bool _dirty;

    bool _delaySaving;

    // Used during searching to make it possible to search for Jesper & None
    mutable QMap<QString, StringSet > _matched;
};

}

#endif /* IMAGEINFO_H */


/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

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

class ImageInfo {

public:
    enum ExifMode { Init, Time };

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

    void readExif(const QString& fullPath, ExifMode mode);

    void rotate( int degrees );
    int angle() const;

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

    bool imageOnDisk() const { return _imageOnDisk; }
    void setImageOnDisk( bool b ) {_imageOnDisk = b; }
    QString MD5Sum() const { return _md5sum; }
    void setMD5Sum( const QString& sum ) { _md5sum = sum; }

    void setLocked( bool );
    bool isLocked() const;

    QImage load( int width = -1, int height = -1 ) const;

    bool isNull() const { return _null; }

protected:
    bool loadJPEG(QImage* image, const QString& fileName ) const;
    bool isJPEG( const QString& fileName ) const;

    friend class ImageRow;
    void setImage( const QImage& image ) { _importImage = image; }
    QImage _importImage;

private:
    QString _fileName;
    QString _label;
    QString _description;
    ImageDate _startDate, _endDate;
    QMap<QString, QStringList> _options;
    int _angle;
    bool _visible;
    DrawList _drawList;
    bool _imageOnDisk; // true if the image is available on disk
    QString _md5sum;
    bool _null;

    // Cache information
    bool _locked;
};

typedef QPtrList<ImageInfo> ImageInfoList;
typedef QPtrListIterator<ImageInfo> ImageInfoListIterator;

#endif /* IMAGEINFO_H */


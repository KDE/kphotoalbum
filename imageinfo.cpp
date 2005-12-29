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

extern "C" {
#define XMD_H // prevent INT32 clash from jpeglib
#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
}

#include "imageinfo.h"
#include <qfileinfo.h>
#include <qimage.h>
#include <qdom.h>
#include "options.h"
#include "util.h"
#include <kdebug.h>
#include <qwmatrix.h>
#include <qvariant.h>
#include <kmessagebox.h>
#include <klocale.h>
#include "imagedb.h"
#include "categorycollection.h"
#include "fileinfo.h"
#include <qstringlist.h>
#include "membermap.h"
#include <kcmdlineargs.h>
#include "XMLDB/XMLCategory.h"
#include <config.h>
#include "Exif/Database.h"

ImageInfo::ImageInfo() :_null( true ), _locked( false )
{
}

ImageInfo::ImageInfo( const QString& fileName )
    :  _imageOnDisk( YesOnDisk ), _null( false ), _size( -1, -1 ), _locked( false )
{
    QString fullPath = Options::instance()->imageDirectory()+ fileName;
    QFileInfo fi( Options::instance()->imageDirectory() + fileName );
    _label = fi.baseName( true );
    _angle = 0;

    setFileName( fileName);

    // Read EXIF information
    readExif(fullPath, EXIFMODE_INIT);
}

void ImageInfo::setLabel( const QString& desc )
{
    _label = desc;
}

QString ImageInfo::label() const
{
    return _label;
}

void ImageInfo::setDescription( const QString& desc )
{
    _description = desc;
}

QString ImageInfo::description() const
{
    return _description;
}


void ImageInfo::setOption( const QString& key, const QStringList& value )
{
    _options[key] = value;
}

void ImageInfo::addOption( const QString& key, const QStringList& value )
{
    for( QStringList::ConstIterator it = value.begin(); it != value.end(); ++it ) {
        if (! _options[key].contains( *it ) )
            _options[key] += *it;
    }
}

bool ImageInfo::hasOption( const QString& key, const QString& value )
{
    return _options[key].contains(value);
}

QStringList ImageInfo::itemsOfCategory( const QString& key ) const
{
    return _options[key];
}

void ImageInfo::renameItem( const QString& key, const QString& oldValue, const QString& newValue )
{
    QStringList& list = _options[key];
    QStringList::Iterator it = list.find( oldValue );
    if ( it != list.end() ) {
        list.remove( it );
        list.append( newValue );
    }
}

QString ImageInfo::fileName( bool relative ) const
{
    if (relative)
        return _fileName;
    else
        return  Options::instance()->imageDirectory() + _fileName;
}

void ImageInfo::setFileName( const QString& relativeFileName )
{
    _fileName = relativeFileName;
    _imageOnDisk = Unchecked;
    QString folderName = Util::relativeFolderName( _fileName );
    _options.insert( QString::fromLatin1( "Folder") , QStringList( folderName ) );
    ImageDB::instance()->categoryCollection()->categoryForName(QString::fromLatin1("Folder"))->addItem( folderName );
}


QDomElement ImageInfo::save( QDomDocument doc )
{
    QDomElement elm = doc.createElement( QString::fromLatin1("image") );
    elm.setAttribute( QString::fromLatin1("file"),  fileName( true ) );
    elm.setAttribute( QString::fromLatin1("label"),  _label );
    elm.setAttribute( QString::fromLatin1("description"), _description );

    if ( KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) ) {
        elm.setAttribute( QString::fromLatin1("yearFrom"), _date.start().date().year() );
        elm.setAttribute( QString::fromLatin1("monthFrom"),  _date.start().date().month() );
        elm.setAttribute( QString::fromLatin1("dayFrom"),  _date.start().date().day() );
        elm.setAttribute( QString::fromLatin1("hourFrom"), _date.start().time().hour() );
        elm.setAttribute( QString::fromLatin1("minuteFrom"), _date.start().time().minute() );
        elm.setAttribute( QString::fromLatin1("secondFrom"), _date.start().time().second() );

        elm.setAttribute( QString::fromLatin1("yearTo"), _date.end().date().year() );
        elm.setAttribute( QString::fromLatin1("monthTo"),  _date.end().date().month() );
        elm.setAttribute( QString::fromLatin1("dayTo"),  _date.end().date().day() );

    }
    else {
        elm.setAttribute( QString::fromLatin1( "startDate" ), _date.start().toString(Qt::ISODate) );
        elm.setAttribute( QString::fromLatin1( "endDate" ), _date.end().toString(Qt::ISODate) );
    }

    elm.setAttribute( QString::fromLatin1("angle"),  _angle );
    elm.setAttribute( QString::fromLatin1( "md5sum" ), _md5sum );
    elm.setAttribute( QString::fromLatin1( "width" ), _size.width() );
    elm.setAttribute( QString::fromLatin1( "height" ), _size.height() );

    if ( _options.count() != 0 ) {
        if ( Options::instance()->useCompressedIndexXML() && !KCmdLineArgs::parsedArgs()->isSet( "export-in-2.1-format" ) ) {
            writeCategoriesCompressed( elm, _options );
        }
        else {
            QDomElement top = doc.createElement( QString::fromLatin1("options") );
            bool any = writeCategories( doc, top, _options );
            if ( any )
                elm.appendChild( top );
        }
    }

    _drawList.save( doc, elm );
    return elm;
}

bool ImageInfo::writeCategories( QDomDocument doc, QDomElement elm, QMap<QString, QStringList>& options )
{
    bool anyAtAll = false;
    QStringList grps = ImageDB::instance()->categoryCollection()->categoryNames();
    for( QStringList::Iterator it = grps.begin(); it != grps.end(); ++it ) {
        QDomElement opt = doc.createElement( QString::fromLatin1("option") );
        QString name = *it;
        opt.setAttribute( QString::fromLatin1("name"),  name );

        QStringList list = options[name];
        bool any = false;
        for( QStringList::Iterator it2 = list.begin(); it2 != list.end(); ++it2 ) {
            QDomElement val = doc.createElement( QString::fromLatin1("value") );
            val.setAttribute( QString::fromLatin1("value"), *it2 );
            opt.appendChild( val );
            any = true;
            anyAtAll = true;
        }
        if ( any )
            elm.appendChild( opt );
    }
    return anyAtAll;
}

void ImageInfo::writeCategoriesCompressed( QDomElement& elm, QMap<QString, QStringList>& categories )
{
    QValueList<CategoryPtr> categoryList = ImageDB::instance()->categoryCollection()->categories();
    for( QValueList<CategoryPtr>::Iterator categoryIt = categoryList.begin(); categoryIt != categoryList.end(); ++categoryIt ) {
        QString categoryName = (*categoryIt)->name();
        QStringList items = categories[categoryName];
        if ( !items.isEmpty() ) {
            QStringList idList;
            for( QStringList::Iterator itemIt = items.begin(); itemIt != items.end(); ++itemIt ) {
                int id = static_cast<XMLDB::XMLCategory*>((*categoryIt).data())->idForName( *itemIt );
                idList.append( QString::number( id ) );
            }
            elm.setAttribute( categoryName, idList.join( QString::fromLatin1( "," ) ) );
        }
    }
}

void ImageInfo::rotate( int degrees )
{
    _angle += degrees + 360;
    _angle = _angle % 360;
}

int ImageInfo::angle() const
{
    return _angle;
}

void ImageInfo::setAngle( int angle )
{
    _angle = angle;
}


void ImageInfo::setDate( const ImageDate& date )
{
    _date = date;
}

ImageDate& ImageInfo::date()
{
    return _date;
}

ImageDate ImageInfo::date() const
{
    return _date;
}

bool ImageInfo::operator!=( const ImageInfo& other )
{
    return !(*this == other);
}

bool ImageInfo::operator==( const ImageInfo& other )
{
    bool changed =
        ( _fileName != other._fileName ||
          _label != other._label ||
          ( !_description.isEmpty() && !other._description.isEmpty() && _description != other._description ) || // one might be isNull.
          _date != other._date ||
          _angle != other._angle);
    if ( !changed ) {
        QStringList keys = ImageDB::instance()->categoryCollection()->categoryNames();
        for( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
            _options[*it].sort();
            QStringList otherList = other._options[*it];
            otherList.sort();
            changed |= _options[*it] != otherList;
        }
    }
    return !changed;
}

void ImageInfo::removeOption( const QString& key, const QString& value )
{
    _options[key].remove( value );
}

DrawList ImageInfo::drawList() const
{
    return _drawList;
}

void ImageInfo::setDrawList( const DrawList& list )
{
    _drawList = list;
}

void ImageInfo::renameCategory( const QString& oldName, const QString& newName )
{
    _options[newName] = _options[oldName];
    _options.erase(oldName);
}

void ImageInfo::setLocked( bool locked )
{
    _locked = locked;
}

bool ImageInfo::isLocked() const
{
    return _locked;
}

void ImageInfo::readExif(const QString& fullPath, int mode)
{
    FileInfo exifInfo = FileInfo::read( fullPath );

    // Date
    if ( (mode & EXIFMODE_DATE) && ( (mode & EXIFMODE_FORCE) || Options::instance()->trustTimeStamps() ) ) {
        QDate date = exifInfo.date();
        QTime time = exifInfo.time();
        if ( date.isValid() ) {
            if ( time.isValid() )
                _date = QDateTime( date, time );
            else
                _date = ImageDate( QDateTime( date, QTime( 0,0,0 ) ), QDateTime( date, QTime( 23, 59, 59 ) ) );
        }
    }

    // Orientation
    if ( (mode & EXIFMODE_ORIENTATION) && Options::instance()->useEXIFRotate() )
        _angle = exifInfo.angle();

    // Description
    if ( (mode & EXIFMODE_DESCRIPTION) && Options::instance()->useEXIFComments() )
        _description = exifInfo.description();

    // Database update
    if ( mode & EXIFMODE_DATABASE_UPDATE ) {
#ifdef HASEXIV2
        Exif::Database::instance()->add( fullPath );
#endif
    }
}


QStringList ImageInfo::availableCategories() const
{
    return _options.keys();
}

void ImageInfo::clearMatched() const
{
    _matched.clear();
}

void ImageInfo::setMatched( const QString& category, const QString& value ) const
{
    _matched[category].append( value );
    const MemberMap& map = ImageDB::instance()->memberMap();
    QStringList members = map.members( category, value, true );
    _matched[category] += members;
}

// Returns whether all tokens for the given image are matches by the search
// example: returns true if all people on an image is in the search, i.e.
// it is only true if there are no persons on the image that are not explicit searched for.
bool ImageInfo::allMatched( const QString& category )
{
    QStringList list = itemsOfCategory( category );
    for( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
        if ( !_matched[category].contains( *it ) )
            return false;
    }
    return true;
}

bool ImageInfo::imageOnDisk() const
{
    if ( _imageOnDisk == Unchecked ) {
        QFileInfo fi( fileName() );
        _imageOnDisk = (fi.exists() ? YesOnDisk : NoNotOnDisk);
    }
    return _imageOnDisk == YesOnDisk;
}

ImageDateRange ImageInfo::dateRange() const
{
    return ImageDateRange( _date );
}

QSize ImageInfo::size() const
{
    return _size;
}

void ImageInfo::setSize( const QSize& size )
{
    _size = size;
}

bool ImageInfo::imageOnDisk( const QString& fileName )
{
    QFileInfo fi( fileName );
    return fi.exists();
}

ImageInfo::ImageInfo( const QString& fileName,
                      const QString& label,
                      const QString& description,
                      const ImageDate& date,
                      int angle,
                      const QString& md5sum,
                      const QSize& size )
{
    _fileName = fileName;
    _label =label;
    _description =description;
    _date = date;
    _angle =angle;
    _md5sum =md5sum;
    _size = size;
    _imageOnDisk = Unchecked;
    _locked = false;
}

ImageInfo& ImageInfo::operator=( const ImageInfo& other )
{
    _fileName = other._fileName;
    _label = other._label;
    _description = other._description;
    _date = other._date;
    _options = other._options;
    _angle = other._angle;
    _drawList = other._drawList;
    _imageOnDisk = other._imageOnDisk;
    _md5sum = other._md5sum;
    _null = other._null;
    _size = other._size;

    return *this;
}

void ImageInfo::addDrawing( const QDomElement& elm )
{
    _drawList.load( elm );
}

#include "infobox.moc"

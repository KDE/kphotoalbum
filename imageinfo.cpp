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

bool ImageInfo::_anyImageWithEmptySize = false;

ImageInfo::ImageInfo() :_null( true )
{
}

ImageInfo::ImageInfo( const QString& fileName )
    :  _visible( true ), _imageOnDisk( YesOnDisk ), _null( false ), _size( -1, -1 ), _locked( false )
{
    QString fullPath = Options::instance()->imageDirectory()+ fileName;
    QFileInfo fi( Options::instance()->imageDirectory() + fileName );
    _label = fi.baseName( true );
    _angle = 0;

    setFileName( fileName);

    // Read EXIF information
    readExif(fullPath, EXIFMODE_INIT);
}

ImageInfo::ImageInfo( const QString& fileName, QDomElement elm )
    :  _visible( true ), _null( false ), _locked( false )
{
    QFileInfo fi( Options::instance()->imageDirectory()+ fileName );
    setFileName( fileName );
    _label = elm.attribute( QString::fromLatin1("label"),  _label );
    _description = elm.attribute( QString::fromLatin1("description") );

    int yearFrom = 0, monthFrom = 0,  dayFrom = 0, yearTo = 0, monthTo = 0,  dayTo = 0, hourFrom = -1, minuteFrom = -1, secondFrom = -1;

    _startDate.setYear( elm.attribute( QString::fromLatin1("yearFrom"), QString::number( yearFrom) ).toInt() );
    _startDate.setMonth( elm.attribute( QString::fromLatin1("monthFrom"), QString::number(monthFrom) ).toInt() );
    _startDate.setDay( elm.attribute( QString::fromLatin1("dayFrom"), QString::number(dayFrom) ).toInt() );
    _startDate.setHour( elm.attribute( QString::fromLatin1("hourFrom"), QString::number(hourFrom) ).toInt() );
    _startDate.setMinute( elm.attribute( QString::fromLatin1("minuteFrom"), QString::number(minuteFrom) ).toInt() );
    _startDate.setSecond( elm.attribute( QString::fromLatin1("secondFrom"), QString::number(secondFrom) ).toInt() );

    _endDate.setYear( elm.attribute( QString::fromLatin1("yearTo"), QString::number(yearTo) ).toInt() );
    _endDate.setMonth( elm.attribute( QString::fromLatin1("monthTo"), QString::number(monthTo) ).toInt() );
    _endDate.setDay( elm.attribute( QString::fromLatin1("dayTo"), QString::number(dayTo) ).toInt() );

    _angle = elm.attribute( QString::fromLatin1("angle"), QString::fromLatin1("0") ).toInt();
    _md5sum = elm.attribute( QString::fromLatin1( "md5sum" ) );

    _anyImageWithEmptySize |= !elm.hasAttribute( QString::fromLatin1( "width" ) );

    int w = elm.attribute( QString::fromLatin1( "width" ), QString::fromLatin1( "-1" ) ).toInt();
    int h = elm.attribute( QString::fromLatin1( "height" ), QString::fromLatin1( "-1" ) ).toInt();
    _size = QSize( w,h );

    for ( QDomNode child = elm.firstChild(); !child.isNull(); child = child.nextSibling() ) {
        if ( child.isElement() ) {
            QDomElement childElm = child.toElement();
            if ( childElm.tagName() == QString::fromLatin1( "options" ) ) {
                Util::readOptions( childElm, &_options, 0 );
            }
            else if ( childElm.tagName() == QString::fromLatin1( "drawings" ) ) {
                _drawList.load( childElm );
            }
            else {
                KMessageBox::error( 0, i18n("<qt><p>Unknown tag %1, while reading configuration file.</p>"
                                            "<p>Expected one of: Options, Drawings</p></qt>" ).arg( childElm.tagName() ) );
            }
        }
    }
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

QStringList ImageInfo::optionValue( const QString& key ) const
{
    return _options[key];
}

void ImageInfo::renameOption( const QString& key, const QString& oldValue, const QString& newValue )
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
    Options::instance()->addOption( QString::fromLatin1("Folder"), folderName );
}


QDomElement ImageInfo::save( QDomDocument doc )
{
    QDomElement elm = doc.createElement( QString::fromLatin1("image") );
    elm.setAttribute( QString::fromLatin1("file"),  fileName( true ) );
    elm.setAttribute( QString::fromLatin1("label"),  _label );
    elm.setAttribute( QString::fromLatin1("description"), _description );

    elm.setAttribute( QString::fromLatin1("yearFrom"), _startDate.year() );
    elm.setAttribute( QString::fromLatin1("monthFrom"),  _startDate.month() );
    elm.setAttribute( QString::fromLatin1("dayFrom"),  _startDate.day() );
    elm.setAttribute( QString::fromLatin1("hourFrom"), _startDate.hour() );
    elm.setAttribute( QString::fromLatin1("minuteFrom"), _startDate.minute() );
    elm.setAttribute( QString::fromLatin1("secondFrom"), _startDate.second() );

    elm.setAttribute( QString::fromLatin1("yearTo"), _endDate.year() );
    elm.setAttribute( QString::fromLatin1("monthTo"),  _endDate.month() );
    elm.setAttribute( QString::fromLatin1("dayTo"),  _endDate.day() );

    elm.setAttribute( QString::fromLatin1("angle"),  _angle );
    elm.setAttribute( QString::fromLatin1( "md5sum" ), _md5sum );
    elm.setAttribute( QString::fromLatin1( "width" ), _size.width() );
    elm.setAttribute( QString::fromLatin1( "height" ), _size.height() );

    if ( _options.count() != 0 ) {
        QDomElement top = doc.createElement( QString::fromLatin1("options") );
        bool any = Util::writeOptions( doc, top, _options, 0 );
        if ( any )
            elm.appendChild( top );
    }

    _drawList.save( doc, elm );
    return elm;
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


void ImageInfo::setStartDate( const ImageDate& date )
{
    _startDate = date;
}

void ImageInfo::setEndDate( const ImageDate& date )
{
    _endDate = date;
}

ImageDate& ImageInfo::startDate()
{
    return _startDate;
}

ImageDate& ImageInfo::endDate()
{
    return _endDate;
}

void ImageInfo::setVisible( bool b )
{
    _visible = b;
}

bool ImageInfo::visible() const
{
    return _visible;
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
          _startDate != other._startDate ||
          _endDate != other._endDate ||
          _angle != other._angle);
    if ( !changed ) {
        QStringList keys = CategoryCollection::instance()->categoryNames();
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

void ImageInfo::renameOptionGroup( const QString& oldName, const QString& newName )
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
    QFileInfo fi( fullPath );
    FileInfo exifInfo = FileInfo::read( fullPath );
    static bool hasShownWarning = false;
    bool foundInExif = 0;
    if ( exifInfo.isEmpty() && !hasShownWarning ) {
        hasShownWarning = true;
        KMessageBox::information( 0, i18n("<qt><p><b>KimDaBa was unable to read EXIF information.</b></p>"
                                          "<p>EXIF information is meta information about the image stored in JPEG files. "
                                          "KimDaBa tries to read the date, orientation and description from EXIF.</p>"
                                          "<p>However, KimDaBa was unable to read information from %1. This may "
                                          "either be because the file does not contain any EXIF information, or "
                                          "because you did not install the package kde-graphics.</p></qt>").arg( fullPath ),
                                  i18n("Unable to Read EXIF Information"), QString::fromLatin1("UnableToReadEXIFInformation") );
    }

    //Time
    if ( mode & EXIFMODE_TIME ) {
        if ( (mode & EXIFMODE_FORCE) || Options::instance()->trustTimeStamps() ) {
            QTime time = exifInfo.time( &foundInExif );
            if ( time.isValid() ) {
                if ( foundInExif || (mode & EXIFMODE_FORCE_TIME) )
                    _startDate.setTime( time );
            }
        }
    }

    // Date
    if ( mode & EXIFMODE_DATE ) {
        if ( (mode & EXIFMODE_FORCE) || Options::instance()->trustTimeStamps() ) {
            QDate date = exifInfo.date( &foundInExif );
            if ( date.isValid() ) {
                if ( foundInExif || (mode & EXIFMODE_FORCE_DATE) )
                    _startDate.setDate( date );
                _endDate = ImageDate();
            }
            if ( !foundInExif && !hasShownWarning &&
                 ( _fileName.endsWith( QString::fromLatin1( ".jpg" ) ) ||
                   _fileName.endsWith( QString::fromLatin1( ".jpeg" ) ) ||
                   _fileName.endsWith( QString::fromLatin1( ".JPG" ) ) ||
                   _fileName.endsWith( QString::fromLatin1( ".JPEG" ) ) ) ) {
                hasShownWarning = true;
                KMessageBox::information( 0, i18n("<qt><p><b>KimDaBa was unable to read the date from the EXIF information.</b></p>"
                                                  "<p>EXIF information is meta information about the image stored in JPEG files. "
                                                  "KimDaBa tries to read the date, orientation and description from EXIF.</p>"
                                                  "<p>However, KimDaBa was unable to read date information from %1. This may "
                                                  "either be because the file did not contain any EXIF information, or "
                                                  "because you did not install the package kde-graphics.</p></qt>").arg( fullPath ),
                                          i18n("Unable to Read Date From EXIF Information"),
                                          QString::fromLatin1("UnableToReadEXIFInformation") );
            }
        }
    }

    // Orientation
    if ( mode & EXIFMODE_ORIENTATION ) {
        if ( Options::instance()->useEXIFRotate() ) {
            int angle = exifInfo.angle( &foundInExif );
            if ( foundInExif )
                _angle = angle;
        }
    }

    // Description
    if ( mode & EXIFMODE_DESCRIPTION ) {
        if ( Options::instance()->useEXIFComments() ) {
            QString desc = exifInfo.description( &foundInExif );
            if ( foundInExif )
                _description = exifInfo.description();
        }
    }
}


QStringList ImageInfo::availableOptionGroups() const
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
    const MemberMap& map = Options::instance()->memberMap();
    QStringList members = map.members( category, value, true );
    _matched[category] += members;
}

// Returns whether all tokens for the given image are matches by the search
// example: returns true if all people on an image is in the search, i.e.
// it is only true if there are no persons on the image that are not explicit searched for.
bool ImageInfo::allMatched( const QString& category )
{
    QStringList list = optionValue( category );
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

void ImageInfo::setImageOnDisk( bool b )
{
    _imageOnDisk = (b ? YesOnDisk : NoNotOnDisk);
}

ImageDateRange ImageInfo::dateRange() const
{
    return ImageDateRange( _startDate, _endDate );
}

QSize ImageInfo::size() const
{
    return _size;
}

void ImageInfo::setSize( const QSize& size )
{
    _size = size;
}

#include "infobox.moc"

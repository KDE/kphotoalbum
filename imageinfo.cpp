/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
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


ImageInfo::ImageInfo()
{
}

ImageInfo::ImageInfo( const QString& fileName )
    : _fileName( fileName ), _visible( true ), _imageOnDisk( true )
{
    QFileInfo fi( Options::instance()->imageDirectory()+ QString::fromLatin1("/") + fileName );
    _label = fi.baseName();
    _angle = 0;

    if ( Options::instance()->trustTimeStamps() )  {
        QDate date = fi.lastModified().date();
        _startDate.setYear(date.year());
        _startDate.setMonth(date.month());
        _startDate.setDay(date.day());
    }
}

ImageInfo::ImageInfo( const QString& fileName, QDomElement elm )
    : _fileName( fileName ), _visible( true )
{
    QFileInfo fi( Options::instance()->imageDirectory()+ QString::fromLatin1("/") + fileName );
    _imageOnDisk = fi.exists();
    _label = elm.attribute( QString::fromLatin1("label"),  _label );
    _description = elm.attribute( QString::fromLatin1("description") );

    int yearFrom = 0, monthFrom = 0,  dayFrom = 0, yearTo = 0, monthTo = 0,  dayTo = 0;

    _startDate.setYear( elm.attribute( QString::fromLatin1("yearFrom"), QString::number( yearFrom) ).toInt() );
    _startDate.setMonth( elm.attribute( QString::fromLatin1("monthFrom"), QString::number(monthFrom) ).toInt() );
    _startDate.setDay( elm.attribute( QString::fromLatin1("dayFrom"), QString::number(dayFrom) ).toInt() );

    _endDate.setYear( elm.attribute( QString::fromLatin1("yearTo"), QString::number(yearTo) ).toInt() );
    _endDate.setMonth( elm.attribute( QString::fromLatin1("monthTo"), QString::number(monthTo) ).toInt() );
    _endDate.setDay( elm.attribute( QString::fromLatin1("dayTo"), QString::number(dayTo) ).toInt() );

    _angle = elm.attribute( QString::fromLatin1("angle"), QString::fromLatin1("0") ).toInt();
    _md5sum = elm.attribute( QString::fromLatin1( "md5sum" ) );

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
                qWarning("Ups unknown tag '%s'", childElm.tagName().latin1() );
                // PENDING(blackie) Do it the KDE way.
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
        return Options::instance()->imageDirectory() + QString::fromLatin1("/") + _fileName;
}

void ImageInfo::setFileName( const QString& relativeFileName )
{
    _fileName = relativeFileName;
    QFileInfo fi( fileName() );
    _imageOnDisk = fi.exists();

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

    elm.setAttribute( QString::fromLatin1("yearTo"), _endDate.year() );
    elm.setAttribute( QString::fromLatin1("monthTo"),  _endDate.month() );
    elm.setAttribute( QString::fromLatin1("dayTo"),  _endDate.day() );

    elm.setAttribute( QString::fromLatin1("angle"),  _angle );
    elm.setAttribute( QString::fromLatin1( "md5sum" ), _md5sum );

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
        QStringList keys = Options::instance()->optionGroups();
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

void ImageInfo::debug()
{
    for( QMapIterator<QString,QStringList> it= _options.begin(); it != _options.end(); ++it ) {
        kdDebug(50010) << it.key() << ", " << it.data().join( QString::fromLatin1( ", " ) ) << endl;
    }
}

QImage ImageInfo::load( int width, int height ) const
{
    QImage image;
    if ( isJPEG( fileName() ) )
        loadJPEG( &image, fileName() );
    else
        image.load( fileName() );

    if ( _angle != 0 ) {
        QWMatrix matrix;
        matrix.rotate( _angle );
        image = image.xForm( matrix );
    }

    if ( width != -1 && height != -1 )
        image = image.smoothScale( width, height, QImage::ScaleMin );

    return image;

}


// Fudged Fast JPEG decoding code from GWENVIEW (picked out out digikam)

bool ImageInfo::loadJPEG(QImage* image, const QString& fileName ) const
{
    FILE* inputFile=fopen( fileName.latin1(), "rb");
    if(!inputFile) return false;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, inputFile);
    jpeg_read_header(&cinfo, TRUE);

    // Create QImage
    jpeg_start_decompress(&cinfo);

    switch(cinfo.output_components) {
    case 3:
    case 4:
        image->create( cinfo.output_width, cinfo.output_height, 32 );
        break;
    case 1: // B&W image
        image->create( cinfo.output_width, cinfo.output_height, 8, 256 );
        for (int i=0; i<256; i++)
            image->setColor(i, qRgb(i,i,i));
        break;
    default:
        return false;
    }

    uchar** lines = image->jumpTable();
    while (cinfo.output_scanline < cinfo.output_height)
        jpeg_read_scanlines(&cinfo, lines + cinfo.output_scanline,
                            cinfo.output_height);
    jpeg_finish_decompress(&cinfo);

    // Expand 24->32 bpp
    if ( cinfo.output_components == 3 ) {
        for (uint j=0; j<cinfo.output_height; j++) {
            uchar *in = image->scanLine(j) + cinfo.output_width*3;
            QRgb *out = (QRgb*)( image->scanLine(j) );

            for (uint i=cinfo.output_width; i--; ) {
                in-=3;
                out[i] = qRgb(in[0], in[1], in[2]);
            }
        }
    }

    jpeg_destroy_decompress(&cinfo);
    fclose(inputFile);

    return true;
}

bool ImageInfo::isJPEG( const QString& fileName ) const
{
    QString format= QString::fromLocal8Bit( QImageIO::imageFormat( fileName ) );
    return format == QString::fromLocal8Bit( "JPEG" );
}



#include "infobox.moc"

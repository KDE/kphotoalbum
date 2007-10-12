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

#include "ImageInfo.h"
#include <qfileinfo.h>
#include <qdom.h>
#include "Settings/SettingsData.h"
#include "Utilities/Util.h"
#include "DB/ImageDB.h"
#include "DB/CategoryCollection.h"
#include "FileInfo.h"
#include <qstringlist.h>
#include "DB/MemberMap.h"
#include <config.h>
#include "Exif/Database.h"
#include "Utilities/Util.h"
#include "MainWindow/DirtyIndicator.h"
#ifdef HASEXIV2
#   include <exiv2/image.hpp>
#endif

#include <kdebug.h>

using namespace DB;

ImageInfo::ImageInfo() :_null( true ), _locked( false ), _dirty( false ), _delaySaving( false )
{
}

ImageInfo::ImageInfo( const QString& fileName, MediaType type )
    :  _imageOnDisk( YesOnDisk ), _null( false ), _size( -1, -1 ), _type( type ), _locked( false ), _delaySaving( true )
{
    QString fullPath = Settings::SettingsData::instance()->imageDirectory()+ fileName;
    QFileInfo fi( Settings::SettingsData::instance()->imageDirectory() + fileName );
    _label = fi.baseName( true );
    _angle = 0;

    setFileName( fileName);

    // Read EXIF information
    readExif(fullPath, EXIFMODE_INIT);

    _dirty = false;
    _delaySaving = false;
}

/** Change delaying of saving changes.
 *
 * Will save changes when set to false.
 *
 * Use this method to set multiple attributes with only one
 * database operation.
 *
 * Example:
 * \code
 * info.delaySavingChanges(true);
 * info.setLabel("Hello");
 * info.setDescription("Hello world");
 * info.delaySavingChanges(false);
 * \endcode
 *
 * \see saveChanges()
 */
void ImageInfo::delaySavingChanges(bool b)
{
    _delaySaving = b;
    if (!b)
        saveChanges();
}

void ImageInfo::setLabel( const QString& desc )
{
    if (desc != _label)
        _dirty = true;
    _label = desc;
    saveChangesIfNotDelayed();
}

QString ImageInfo::label() const
{
    return _label;
}

void ImageInfo::setDescription( const QString& desc )
{
    if (desc != _description)
        _dirty = true;
    _description = desc;
    saveChangesIfNotDelayed();
}

QString ImageInfo::description() const
{
    return _description;
}


void ImageInfo::setCategoryInfo( const QString& key, const StringSet& value )
{
    // Don't check if really changed, because it's too slow.
    _dirty = true;
    _categoryInfomation[key] = value;
    saveChangesIfNotDelayed();
}

bool ImageInfo::hasCategoryInfo( const QString& key, const QString& value )
{
    return _categoryInfomation[key].contains(value);
}

StringSet ImageInfo::itemsOfCategory( const QString& key ) const
{
    return QStringList(_categoryInfomation[key].toList());
}

void ImageInfo::renameItem( const QString& key, const QString& oldValue, const QString& newValue )
{
    StringSet& set = _categoryInfomation[key];
    StringSet::const_iterator it = set.find( oldValue );
    if ( it != set.end() ) {
        _dirty = true;
        set.erase( it );
        set.insert( newValue );
        saveChangesIfNotDelayed();
    }
}

QString ImageInfo::fileName( bool relative ) const
{
    if (relative)
        return _relativeFileName;
    else
        return _absoluteFileName;
}

void ImageInfo::setFileName( const QString& relativeFileName )
{
    if (relativeFileName != _relativeFileName)
        _dirty = true;
    _relativeFileName = relativeFileName;
    setAbsoluteFileName();
    _imageOnDisk = Unchecked;
    DB::CategoryPtr folderCategory = DB::ImageDB::instance()->categoryCollection()->
        categoryForName(QString::fromLatin1("Folder"));
    if (folderCategory) {
        DB::MemberMap& map = DB::ImageDB::instance()->memberMap();
        createFolderCategoryItem( folderCategory, map );
        //ImageDB::instance()->setMemberMap( map );
    }
    saveChangesIfNotDelayed();
}


void ImageInfo::rotate( int degrees )
{
    if (degrees != 0)
        _dirty = true;
    _angle += degrees + 360;
    _angle = _angle % 360;
    saveChangesIfNotDelayed();
}

int ImageInfo::angle() const
{
    return _angle;
}

void ImageInfo::setAngle( int angle )
{
    if (angle != _angle)
        _dirty = true;
    _angle = angle;
    saveChangesIfNotDelayed();
}


void ImageInfo::setDate( const ImageDate& date )
{
    if (date != _date)
        _dirty = true;
    _date = date;
    saveChangesIfNotDelayed();
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
        ( _relativeFileName != other._relativeFileName ||
          _label != other._label ||
          ( !_description.isEmpty() && !other._description.isEmpty() && _description != other._description ) || // one might be isNull.
          _date != other._date ||
          _angle != other._angle);
    if ( !changed ) {
        QStringList keys = DB::ImageDB::instance()->categoryCollection()->categoryNames();
        for( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it )
            changed |= _categoryInfomation[*it] != other._categoryInfomation[*it];
    }
    return !changed;
}

Viewer::DrawList ImageInfo::drawList() const
{
    return _drawList;
}

void ImageInfo::setDrawList( const Viewer::DrawList& list )
{
    // Can't check if really changed, because DrawList doesn't have operator==
    if (!_drawList.empty() || !list.empty())
        _dirty = true;
    _drawList = list;
    saveChangesIfNotDelayed();
}

void ImageInfo::renameCategory( const QString& oldName, const QString& newName )
{
    _dirty = true;
    _categoryInfomation[newName] = _categoryInfomation[oldName];
    _categoryInfomation.erase(oldName);
    saveChangesIfNotDelayed();
}

void ImageInfo::setLocked( bool locked )
{
    _locked = locked;
}

bool ImageInfo::isLocked() const
{
    return _locked;
}

void ImageInfo::readExif(const QString& fullPath, const int mode)
{
    DB::FileInfo exifInfo = DB::FileInfo::read( fullPath );

    bool oldDelaySaving = _delaySaving;
    delaySavingChanges(true);

    // Label
    if ( mode & EXIFMODE_LABEL )
        setLabel( exifInfo.label() );

    // Date
    if ( mode & EXIFMODE_DATE )
        setDate( exifInfo.dateTime() );

    // Orientation
    if ( mode & EXIFMODE_ORIENTATION )
        setAngle( exifInfo.angle() );

    // Description
    if ( mode & EXIFMODE_DESCRIPTION )
        setDescription( exifInfo.description() );

    // Categories
    if ( mode & EXIFMODE_CATEGORIES ) {
        /*
        FIXME: this code segfaults while the uncommented works. Why?
        for ( QMap<QString, QStringList>::const_iterator it = exifInfo.categories().begin(); it != exifInfo.categories().end(); ++it ) {
            setCategoryInfo( it.key(), it.data() );
        }
        */
        QMap<QString, QStringList> categories = exifInfo.categories();
        QStringList categoryNames = categories.keys();
        for (QStringList::const_iterator it = categoryNames.begin(); it != categoryNames.end(); ++it )
            setCategoryInfo( *it, categories[ *it ] );
    }

    delaySavingChanges(false);
    _delaySaving = oldDelaySaving;

    // Database update
    if ( mode & EXIFMODE_DATABASE_UPDATE ) {
#ifdef HASEXIV2
        Exif::Database::instance()->add( fullPath );
#endif
    }
}

void ImageInfo::writeMetadata( const QString& fullPath, const int mode )
{
#ifdef HASEXIV2
    QMap<Exif::Syncable::Kind,QString> _fieldName, _visibleName;
    QMap<Exif::Syncable::Kind,Exif::Syncable::Header> _header;
    Exif::Syncable::fillTranslationTables( _fieldName, _visibleName, _header);

    bool changed = false;

    try {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open( fullPath.local8Bit().data() );
        if ( !image->good() ) {
            kdWarning() << fullPath << ": Exiv2::Image instance not usable" << endl;
            return;
        }
        image->readMetadata();
        Exiv2::ExifData& exifMap = image->exifData();
        Exiv2::IptcData& iptcMap = image->iptcData();
        std::string keyName;

        if ( mode & EXIFMODE_DATE ) {
            QValueList<Exif::Syncable::Kind> items = Settings::SettingsData::instance()->dateSyncing( true );
            for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin(); ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
                switch ( *it ) {
                    case Exif::Syncable::EXIF_DATETIME:
                    case Exif::Syncable::EXIF_DATETIME_ORIGINAL:
                    case Exif::Syncable::EXIF_DATETIME_DIGITIZED:
                        keyName = _fieldName[*it].ascii();
                        // FIXME: use start() or end()?
                        exifMap[keyName] = _date.start().toString( QString::fromAscii("yyyy:MM:dd hh:mm:ss") ).ascii();
                        changed = true;
                        break;
                    case Exif::Syncable::FILE_MTIME:
                        // A big FIXME:
                        // Well, QFileInfo doesn't have any method for updating
                        // those... :(
                        // Also note that we have to touch it *after* we call
                        // writeMetadata() for obvious reasons :)
                        // Just let it fall through for now...
                    default:
                        kdDebug() << "unknown date field: " << _fieldName[*it] << endl;
                }
            }
        }

        if ( mode & EXIFMODE_ORIENTATION ) {
            QValueList<Exif::Syncable::Kind> items = Settings::SettingsData::instance()->orientationSyncing( true );
            for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin(); ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
                switch ( *it ) {
                    case Exif::Syncable::EXIF_ORIENTATION:
                    { // we need a new block for local variables
                        int orientation;
                        switch (_angle ) {
                            // for respective values, see http://jpegclub.org/exif_orientation.html
                            // or DB::FileInfo::orientationToAngle()
                            case 0:
                                orientation = 1;
                                break;
                            case 90:
                                orientation = 6;
                                break;
                            case 180:
                                orientation = 3;
                                break;
                            case 270:
                                orientation = 8;
                                break;
                            default:
                                kdDebug() << "unknown _angle: " << _angle << endl;
                                continue;
                        }
                        keyName = _fieldName[*it].ascii();
                        exifMap[keyName] = orientation;
                        changed = true;
                        break;
                    }
                    default:
                        kdDebug() << "unknown orientation field: " << _fieldName[*it] << endl;
                }
            }

        }

        if ( mode & EXIFMODE_LABEL ) {
            QValueList<Exif::Syncable::Kind> items = Settings::SettingsData::instance()->labelSyncing( true );
            for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin(); ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
                switch ( _header[ *it ] ) {
                    case Exif::Syncable::EXIF:
                        keyName = _fieldName[*it].ascii();
                        exifMap[keyName] = Utilities::encodeQString( _label, Settings::SettingsData::instance()->iptcCharset() );
                        changed = true;
                        break;
                    case Exif::Syncable::IPTC:
                        keyName = _fieldName[*it].ascii();
                        iptcMap[keyName] = Utilities::encodeQString( _label, Settings::SettingsData::instance()->iptcCharset() );
                        changed = true;
                        break;
                    case Exif::Syncable::JPEG:
                        if ( *it == Exif::Syncable::JPEG_COMMENT ) {
                            image->setComment( Utilities::encodeQString( _label, Settings::SettingsData::instance()->iptcCharset() ) );
                            changed = true;
                        } else
                            kdDebug() << "Unkown JPEG field " <<  _fieldName[*it] << endl;
                        break;
                    default:
                        kdDebug() << "Unknown label class " << _fieldName[*it] << endl;
                }
            }
        }

        if ( mode & EXIFMODE_DESCRIPTION ) {
            QValueList<Exif::Syncable::Kind> items = Settings::SettingsData::instance()->descriptionSyncing( true );
            for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin(); ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
                switch ( _header[ *it ] ) {
                    case Exif::Syncable::EXIF:
                        keyName = _fieldName[*it].ascii();
                        exifMap[keyName] = Utilities::encodeQString( _description, Settings::SettingsData::instance()->iptcCharset() );
                        changed = true;
                        break;
                    case Exif::Syncable::IPTC:
                        keyName = _fieldName[*it].ascii();
                        iptcMap[keyName] = Utilities::encodeQString( _description, Settings::SettingsData::instance()->iptcCharset() );
                        changed = true;
                        break;
                    case Exif::Syncable::JPEG:
                        if ( *it == Exif::Syncable::JPEG_COMMENT ) {
                            image->setComment( Utilities::encodeQString( _description, Settings::SettingsData::instance()->iptcCharset() ) );
                            changed = true;
                        } else
                            kdDebug() << "Unkown JPEG field " <<  _fieldName[*it] << endl;
                        break;
                    default:
                        kdDebug() << "Unknown description class " << _fieldName[*it] << endl;
                }
            }
        }

        if ( mode & EXIFMODE_CATEGORIES ) {
            QValueList<DB::CategoryPtr> categories = DB::ImageDB::instance()->categoryCollection()->categories();
                for( QValueList<DB::CategoryPtr>::const_iterator category = categories.begin();
                        category != categories.end(); ++category )
                    if ( !(*category)->isSpecialCategory() ) {
                        QValueList<Exif::Syncable::Kind> items = Settings::SettingsData::instance()->categorySyncingFields( true, (*category)->name() );

                        for (QValueList<Exif::Syncable::Kind>::const_iterator it = items.begin();
                                ( it != items.end() ) && ( *it != Exif::Syncable::STOP ); ++it ) {
                            // get a list of tags (based on supercategory settings)
                            QStringList tags;

                            switch (Settings::SettingsData::instance()->categorySyncingSuperGroups( (*category)->name() ) ) {

                                case Exif::Syncable::MostSpecific:
                                    // only the deepest tag
                                    tags = _categoryInfomation[ (*category)->name() ].toList();
                                    break;

                                case Exif::Syncable::MergeBySlash:
                                case Exif::Syncable::Independent:
                                    // we have to resolve the subcategories here
                                    QMap<QString,StringSet> superGroups = DB::ImageDB::instance()->memberMap().inverseMap( (*category)->name() );
                                    for (StringSet::const_iterator tagIt = _categoryInfomation[(*category)->name()].begin();
                                            tagIt != _categoryInfomation[(*category)->name()].end(); ++tagIt ) {
                                        // resolve supergroups
                                        QStringList queue = *tagIt;
                                        QStringList resolved;
                                        while (!queue.empty()) {
                                            QString item = queue.last();
                                            queue.pop_back();
                                            QMap<QString,StringSet>::const_iterator biggerGroup = superGroups.find( item );
                                            if ( biggerGroup != superGroups.end() )
                                                queue.push_front( (*(*biggerGroup).begin()) );
                                            resolved.push_front( item );
                                        }

                                        if (Settings::SettingsData::instance()->categorySyncingSuperGroups( (*category)->name() ) ==
                                                Exif::Syncable::MergeBySlash )
                                            tags.push_back( resolved.join( QString::fromAscii("/") ) );
                                        else
                                            tags += resolved;
                                    }
                                    break;
                            }

                            if ( tags.empty() )
                                break;
                            else {
                                QString value;

                                if ( Settings::SettingsData::instance()->categorySyncingAddName( (*category)->name() ) )
                                    value = (*category)->name() + QString::fromAscii(":");

                                switch (Settings::SettingsData::instance()->categorySyncingMultiValue( (*category)->name() ) ) {

                                    case Exif::Syncable::SeparateComma:
                                    case Exif::Syncable::SeparateSemicolon:
                                        if (Settings::SettingsData::instance()->categorySyncingMultiValue( (*category)->name() ) == 
                                                Exif::Syncable::SeparateComma )
                                                value += tags.join( QString::fromAscii(", ") );
                                        else
                                                value += tags.join( QString::fromAscii("; ") );

                                        switch ( _header[ *it ] ) {
                                            case Exif::Syncable::EXIF:
                                                {
                                                    keyName = _fieldName[*it].ascii();
                                                    // erase all occurences of this tag
                                                    Exiv2::ExifData::iterator tag;
                                                    while ( ( tag = exifMap.findKey( Exiv2::ExifKey( keyName ) ) ) != exifMap.end() )
                                                        exifMap.erase( tag );
                                                    // FIXME: XP tags are expected in UCS-2...
                                                    exifMap[keyName] = Utilities::encodeQString( value, Settings::SettingsData::instance()->iptcCharset() );
                                                    changed = true;
                                                }
                                                break;
                                            case Exif::Syncable::IPTC:
                                                {
                                                    keyName = _fieldName[*it].ascii();
                                                    Exiv2::IptcData::iterator tag;
                                                    while ( ( tag = iptcMap.findKey( Exiv2::IptcKey( keyName ) ) ) != iptcMap.end() )
                                                        iptcMap.erase( tag );
                                                    iptcMap[keyName] = Utilities::encodeQString( value, Settings::SettingsData::instance()->iptcCharset() );
                                                    changed = true;
                                                }
                                                break;
                                            default:
                                                kdDebug() << "Unknown category class " << _fieldName[*it] << endl;
                                        }
                                        break;

                                    case Exif::Syncable::Repeat:
                                        // include that tag multiple times
                                        for (QStringList::const_iterator tagIt = tags.begin(); tagIt != tags.end(); ++tagIt) {
                                            if ( Settings::SettingsData::instance()->categorySyncingAddName( (*category)->name() ) )
                                                value = (*category)->name() + QString::fromAscii(":") + *tagIt;
                                            else
                                                value = *tagIt;

                                            switch ( _header[ *it ] ) {
                                                case Exif::Syncable::EXIF:
                                                    {
                                                        keyName = _fieldName[*it].ascii();
                                                        Exiv2::Value::AutoPtr v = Exiv2::Value::create( Exiv2::string );
                                                        // FIXME: XP tags are expected in UCS-2...
                                                        v->read( Utilities::encodeQString( value, Settings::SettingsData::instance()->iptcCharset() ) );
                                                        if ( tagIt == tags.begin() ) {
                                                            Exiv2::ExifData::iterator tag;
                                                            while ( ( tag = exifMap.findKey( Exiv2::ExifKey( keyName ) ) ) != exifMap.end() )
                                                                exifMap.erase( tag );
                                                        }
                                                        exifMap.add( Exiv2::ExifKey( keyName ), v.get() );
                                                        changed = true;
                                                    }
                                                    break;
                                                case Exif::Syncable::IPTC:
                                                    {
                                                        keyName = _fieldName[*it].ascii();
                                                        Exiv2::Value::AutoPtr v = Exiv2::Value::create( Exiv2::string );
                                                        v->read( Utilities::encodeQString( value, Settings::SettingsData::instance()->iptcCharset() ) );
                                                        if ( tagIt == tags.begin() ) {
                                                            Exiv2::IptcData::iterator tag;
                                                            while ( ( tag = iptcMap.findKey( Exiv2::IptcKey( keyName ) ) ) != iptcMap.end() )
                                                                iptcMap.erase( tag );
                                                        }
                                                        if ( iptcMap.add( Exiv2::IptcKey( keyName ), v.get() ) )
                                                            kdDebug() << "IPTC field " << _fieldName[*it] << " is not repeatable" << endl;
                                                        changed = true;
                                                    }
                                                    break;
                                                default:
                                                    kdDebug() << "Unknown category class " << _fieldName[*it] << endl;
                                            }
                                        }
                                        break;
                                }
                                changed = true;
                            }
                        }
                    }
        }

        if (changed)
            image->writeMetadata();
    }
    catch( Exiv2::AnyError& e ) {
        // I wonder why kdDebug isn't compatible with std::ostream...
        std::ostringstream out;
        out << e;
        kdDebug() << "Exiv2 exception when writing file " << fullPath << ": " << out.str().data() << endl;
        return;
    }

    if (changed) {
        MD5 oldsum = _md5sum;
        setMD5Sum( Utilities::MD5Sum( fullPath ) );
        if (_md5sum != oldsum )
           MainWindow::DirtyIndicator::markDirty();
    }
#else
    kdDebug() << "ImageInfo::writeMetadata(): can't do much without the Exiv2 library" << endl;
#endif
}


QStringList ImageInfo::availableCategories() const
{
    return _categoryInfomation.keys();
}

void ImageInfo::clearMatched() const
{
    _matched.clear();
}

void ImageInfo::setMatched( const QString& category, const QString& value ) const
{
    _matched[category].insert( value );
    const MemberMap& map = DB::ImageDB::instance()->memberMap();
    QStringList members = map.members( category, value, true );
    _matched[category].insert( members );
}

/**
 * Returns whether all tokens for the given image are matches by the search
 * example: returns true if all people on an image is in the search,
 * i.e. it is only true if there are no persons on the image that are not
 * explicit searched for.
 * This is used when the search includes *None*
 */
bool ImageInfo::allMatched( const QString& category )
{
    const StringSet items = itemsOfCategory( category );
    for( StringSet::const_iterator it = items.begin(); it != items.end(); ++it ) {
        if ( !_matched[category].contains( *it ) )
            return false;
    }

    return true;
}

QSize ImageInfo::size() const
{
    return _size;
}

void ImageInfo::setSize( const QSize& size )
{
    if (size != _size)
        _dirty = true;
    _size = size;
    saveChangesIfNotDelayed();
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
                      const MD5& md5sum,
                      const QSize& size,
                      MediaType type )
{
    _delaySaving = true;
    _relativeFileName = fileName;
    setAbsoluteFileName();
    _label =label;
    _description =description;
    _date = date;
    _angle =angle;
    _md5sum =md5sum;
    _size = size;
    _imageOnDisk = Unchecked;
    _locked = false;
    _null = false;
    _type = type;
    _dirty = true;
    delaySavingChanges(false);
}

ImageInfo& ImageInfo::operator=( const ImageInfo& other )
{
    _relativeFileName = other._relativeFileName;
    setAbsoluteFileName();
    _label = other._label;
    _description = other._description;
    _date = other._date;
    _categoryInfomation = other._categoryInfomation;
    _angle = other._angle;
    _drawList = other._drawList;
    _imageOnDisk = other._imageOnDisk;
    _md5sum = other._md5sum;
    _null = other._null;
    _size = other._size;
    _dirty = other._dirty;

    delaySavingChanges(false);

    return *this;
}

void ImageInfo::addDrawing( const QDomElement& elm )
{
    _dirty = true;
    _drawList.load( elm );
    saveChangesIfNotDelayed();
}

MediaType DB::ImageInfo::mediaType() const
{
    return _type;
}

/**
 * During profiling I found that it took almost 5% of the time during
 * categorizing when browsing, simply to calculate the absolute filename, therefore it is
 * now an instance variable rather than calculated dynamically in
 * fileName().
 */
void DB::ImageInfo::setAbsoluteFileName()
{
    _absoluteFileName = Settings::SettingsData::instance()->imageDirectory() + _relativeFileName;
}

void DB::ImageInfo::createFolderCategoryItem( DB::Category* folderCategory, DB::MemberMap& memberMap )
{
    QString folderName = Utilities::relativeFolderName( _relativeFileName );
    if ( folderName.isNull() )
        return;

    QStringList directories = QStringList::split( QString::fromLatin1( "/" ), folderName, true );

    QString curPath;
    for( QStringList::ConstIterator directoryIt = directories.begin(); directoryIt != directories.end(); ++directoryIt ) {
        if ( curPath.isEmpty() )
            curPath = *directoryIt;
        else {
            QString oldPath = curPath;
            curPath = curPath + QString::fromLatin1( "/" ) + *directoryIt;
            memberMap.addMemberToGroup( folderCategory->name(), oldPath, curPath );
        }
    }

    _categoryInfomation.insert( folderCategory->name() , QStringList( folderName ) );
    folderCategory->addItem( folderName );
}

void DB::ImageInfo::addCategoryInfo( const QString& category, const StringSet& values )
{
    for ( StringSet::const_iterator valueIt = values.begin(); valueIt != values.end(); ++valueIt ) {
        if (! _categoryInfomation[category].contains( *valueIt ) ) {
            _dirty = true;
            _categoryInfomation[category].insert( *valueIt );
        }
    }
    saveChangesIfNotDelayed();
}

void DB::ImageInfo::removeCategoryInfo( const QString& category, const StringSet& values )
{
    for ( StringSet::const_iterator valueIt = values.begin(); valueIt != values.end(); ++valueIt ) {
        if ( _categoryInfomation[category].contains( *valueIt ) ) {
            _dirty = true;
            _categoryInfomation[category].erase(*valueIt);
        }
    }
    saveChangesIfNotDelayed();
}

void DB::ImageInfo::addCategoryInfo( const QString& category, const QString& value )
{
    if (! _categoryInfomation[category].contains( value ) ) {
        _dirty = true;
        _categoryInfomation[category].insert( value );
    }
    saveChangesIfNotDelayed();
}

void DB::ImageInfo::removeCategoryInfo( const QString& category, const QString& value )
{
    if ( _categoryInfomation[category].contains( value ) ) {
        _dirty = true;
        _categoryInfomation[category].erase( value );
    }
    saveChangesIfNotDelayed();
}


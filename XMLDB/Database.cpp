/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Database.h"
#include "Settings/SettingsData.h"
#include <kmessagebox.h>
#include <klocale.h>
#include "Utilities/Util.h"
#include "DB/GroupCounter.h"
#include "Browser/BrowserWidget.h"
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
#include "DB/CategoryCollection.h"
#include "Database.moc"
#include "XMLCategory.h"
#include <ksharedptr.h>
#include "XMLImageDateCollection.h"
#include "FileReader.h"
#include "FileWriter.h"
#ifdef HAVE_EXIV2
#   include "Exif/Database.h"
#endif
#include <DB/FileName.h>

using Utilities::StringSet;

bool XMLDB::Database::_anyImageWithEmptySize = false;
XMLDB::Database::Database( const QString& configFile ):
    _fileName(configFile)
{
    Utilities::checkForBackupFile( configFile );
    FileReader reader( this );
    reader.read( configFile );
    _nextStackId = reader.nextStackId();

    connect( categoryCollection(), SIGNAL( itemRemoved( DB::Category*, const QString& ) ),
             this, SLOT( deleteItem( DB::Category*, const QString& ) ) );
    connect( categoryCollection(), SIGNAL( itemRenamed( DB::Category*, const QString&, const QString& ) ),
             this, SLOT( renameItem( DB::Category*, const QString&, const QString& ) ) );

    connect( categoryCollection(), SIGNAL( itemRemoved( DB::Category*, const QString& ) ),
             &_members, SLOT( deleteItem( DB::Category*, const QString& ) ) );
    connect( categoryCollection(), SIGNAL( itemRenamed( DB::Category*, const QString&, const QString& ) ),
             &_members, SLOT( renameItem( DB::Category*, const QString&, const QString& ) ) );
}

uint XMLDB::Database::totalCount() const
{
    return _images.count();
}

/**
 * I was considering merging the two calls to this method (one for images, one for video), but then I
 * realized that all the work is really done after the check for whether the given
 * imageInfo is of the right type, and as a match can't be both, this really
 * would buy me nothing.
 */
QMap<QString,uint> XMLDB::Database::classify( const DB::ImageSearchInfo& info, const QString &category, DB::MediaType typemask )
{
    QMap<QString, uint> map;
    DB::GroupCounter counter( category );
    Utilities::StringSet alreadyMatched = info.findAlreadyMatched( category );

    DB::ImageSearchInfo noMatchInfo = info;
    QString currentMatchTxt = noMatchInfo.categoryMatchText( category );
    if ( currentMatchTxt.isEmpty() )
        noMatchInfo.setCategoryMatchText( category, DB::ImageDB::NONE() );
    else
        noMatchInfo.setCategoryMatchText( category, QString::fromLatin1( "%1 & %2" ).arg(currentMatchTxt).arg(DB::ImageDB::NONE()) );

    // Iterate through the whole database of images.
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        bool match = ( (*it)->mediaType() & typemask ) && !(*it)->isLocked() && info.match( *it ) && rangeInclude( *it );
        if ( match ) { // If the given image is currently matched.

            // Now iterate through all the categories the current image
            // contains, and increase them in the map mapping from category
            // to count.
            StringSet items = (*it)->itemsOfCategory(category);
            counter.count( items );
            for( StringSet::const_iterator it2 = items.begin(); it2 != items.end(); ++it2 ) {
                if ( !alreadyMatched.contains(*it2) ) // We do not want to match "Jesper & Jesper"
                    map[*it2]++;
            }

            // Find those with no other matches
            if ( noMatchInfo.match( *it ) )
                map[DB::ImageDB::NONE()]++;
        }
    }

    QMap<QString,uint> groups = counter.result();
    for( QMap<QString,uint>::iterator it= groups.begin(); it != groups.end(); ++it ) {
        map[it.key()] = it.value();
    }

    return map;
}

void XMLDB::Database::renameCategory( const QString& oldName, const QString newName )
{
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        (*it)->renameCategory( oldName, newName );
    }
}

void XMLDB::Database::addToBlockList(const DB::FileNameList& list)
{
    _blockList.append(list);
    deleteList( list );
}

void XMLDB::Database::deleteList(const DB::FileNameList& list)
{
    Q_FOREACH(const DB::FileName& fileName, list) {
        DB::ImageInfoPtr inf = fileName.info();
        StackMap::iterator found = _stackMap.find(inf->stackId());
        if ( inf->isStacked() && found != _stackMap.end() ) {
            const DB::FileNameList origCache = found.value();
            DB::FileNameList newCache;
            Q_FOREACH(const DB::FileName& cacheName, origCache) {
                if (fileName != cacheName)
                    newCache.append(cacheName);
            }
            if (newCache.size() <= 1) {
                // we're destroying a stack
                Q_FOREACH(const DB::FileName& cacheName, newCache) {
                    DB::ImageInfoPtr cacheInf = cacheName.info();
                    cacheInf->setStackId(0);
                    cacheInf->setStackOrder(0);
                }
                _stackMap.remove( inf->stackId() );
            } else {
                _stackMap.insert(inf->stackId(), newCache);
            }
        }
#ifdef HAVE_EXIV2
        Exif::Database::instance()->remove( inf->fileName() );
#endif
        _images.remove( inf );
    }
    emit totalChanged( _images.count() );
    emit imagesDeleted(list);
}

void XMLDB::Database::renameItem( DB::Category* category, const QString& oldName, const QString& newName )
{
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        (*it)->renameItem( category->name(), oldName, newName );
    }
}

void XMLDB::Database::deleteItem( DB::Category* category, const QString& value )
{
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        (*it)->removeCategoryInfo( category->name(), value );
    }
}

void XMLDB::Database::lockDB( bool lock, bool exclude  )
{
    DB::ImageSearchInfo info = Settings::SettingsData::instance()->currentLock();
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        if ( lock ) {
            bool match = info.match( *it );
            if ( !exclude )
                match = !match;
            (*it)->setLocked( match );
        }
        else
            (*it)->setLocked( false );
    }
}


void XMLDB::Database::addImages( const DB::ImageInfoList& images )
{
    // FIXME: merge stack information
    DB::ImageInfoList newImages = images.sort();
    if ( _images.count() == 0 ) {
        // case 1: The existing imagelist is empty.
        _images = newImages;
    }
    else if ( newImages.count() == 0 ) {
        // case 2: No images to merge in - that's easy ;-)
        return;
    }
    else if ( newImages.first()->date().start() > _images.last()->date().start() ) {
        // case 2: The new list is later than the existsing
        _images.appendList(newImages);
    }
    else if ( _images.isSorted() ) {
        // case 3: The lists overlaps, and the existsing list is sorted
        _images.mergeIn( newImages );
    }
    else{
        // case 4: The lists overlaps, and the existsing list is not sorted in the overlapping range.
        _images.appendList( newImages );
    }

    for( DB::ImageInfoListConstIterator imageIt = images.constBegin(); imageIt != images.constEnd(); ++imageIt ) {
        DB::ImageInfoPtr info = *imageIt;
        info->addCategoryInfo( QString::fromLatin1( "Media Type" ),
                               info->mediaType() == DB::Image ? QString::fromLatin1( "Image" ) : QString::fromLatin1( "Video" ) );
    }

    emit totalChanged( _images.count() );
    emit dirty();
}

void XMLDB::Database::renameImage( DB::ImageInfoPtr info, const DB::FileName& newName )
{
    info->delaySavingChanges(false);
    info->setFileName(newName);
}

DB::ImageInfoPtr XMLDB::Database::info( const DB::FileName& fileName ) const
{
    typedef QHash<QString, DB::ImageInfoPtr > Cache;
    static Cache fileMap;

    if ( fileName.isNull() )
        return DB::ImageInfoPtr();

    const QString name = fileName.absolute();

    Cache::iterator lookup = fileMap.find(name);

    if ( lookup != fileMap.end() )
        return *lookup;
    else {
        fileMap.clear();
        for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
            fileMap.insert( (*it)->fileName().absolute(), *it );
        }
        if ( fileMap.contains( name ) )
            return fileMap[ name ];
    }

    if (fileMap.contains( name ))
        return DB::ImageInfoPtr(new DB::ImageInfo);
    else
        return DB::ImageInfoPtr();
}

bool XMLDB::Database::rangeInclude( DB::ImageInfoPtr info ) const
{
    if (_selectionRange.start().isNull() )
        return true;

    DB::ImageDate::MatchType tp = info->date().isIncludedIn( _selectionRange );
    if ( _includeFuzzyCounts )
        return ( tp == DB::ImageDate::ExactMatch || tp == DB::ImageDate::RangeMatch );
    else
        return ( tp == DB::ImageDate::ExactMatch );
}


DB::MemberMap& XMLDB::Database::memberMap()
{
    return _members;
}


void XMLDB::Database::save( const QString& fileName, bool isAutoSave )
{
    FileWriter saver( this );
    saver.save( fileName, isAutoSave );
}


DB::MD5Map* XMLDB::Database::md5Map()
{
    return &_md5map;
}

bool XMLDB::Database::isBlocking( const DB::FileName& fileName )
{
    return _blockList.contains( fileName );
}


DB::FileNameList XMLDB::Database::images()
{
    return _images.files();
}

DB::FileNameList XMLDB::Database::search(
        const DB::ImageSearchInfo& info,
        bool requireOnDisk) const
{
    return searchPrivate( info, requireOnDisk, true );
}

DB::FileNameList XMLDB::Database::searchPrivate(
        const DB::ImageSearchInfo& info,
        bool requireOnDisk,
        bool onlyItemsMatchingRange) const
{
    // When searching for images counts for the datebar, we want matches outside the range too.
    // When searching for images for the thumbnail view, we only want matches inside the range.
    DB::FileNameList result;
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && ( !onlyItemsMatchingRange || rangeInclude( *it ));
        match &= !requireOnDisk || DB::ImageInfo::imageOnDisk( (*it)->fileName() );

        if (match)
            result.append((*it)->fileName());
    }
    return result;
}

void XMLDB::Database::sortAndMergeBackIn(const DB::FileNameList& fileNameList)
{
    DB::ImageInfoList infoList;
    Q_FOREACH( const DB::FileName fileName, fileNameList )
        infoList.append(fileName.info());
    _images.sortAndMergeBackIn(infoList);
}

DB::CategoryCollection* XMLDB::Database::categoryCollection()
{
    return &_categoryCollection;
}

KSharedPtr<DB::ImageDateCollection> XMLDB::Database::rangeCollection()
{
    return KSharedPtr<DB::ImageDateCollection>(
                new XMLImageDateCollection( searchPrivate( Browser::BrowserWidget::instance()->currentContext(), false, false)));
}

void XMLDB::Database::reorder(
        const DB::FileName& item,
        const DB::FileNameList& selection,
        bool after)
{
    Q_ASSERT(!item.isNull());
    DB::ImageInfoList list = takeImagesFromSelection(selection);
    insertList(item, list, after );
}

// Remove all the images from the database that match the given selection and
// return that sublist.
// This returns the selected and erased images in the order in which they appear
// in the image list itself.
DB::ImageInfoList XMLDB::Database::takeImagesFromSelection(const DB::FileNameList& selection)
{
    DB::ImageInfoList result;
    if (selection.isEmpty())
        return result;

    // iterate over all images (expensive!!) TODO: improve?
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); /**/ ) {
        const DB::FileName imagefile = (*it)->fileName();
        DB::FileNameList::ConstIterator si = selection.begin();
        // for each image, iterate over selection, break on match
        for ( /**/; si != selection.end(); ++si ) {
            const DB::FileName file = *si;
            if ( imagefile == file ) {
                break;
            }
        }
        // if image is not in selection, simply advance to next, if not add to result and erase
        if (si == selection.end()) {
            ++it;
        } else {
            result << *it;
            it = _images.erase(it);
        }
        // if all images from selection are in result (size of lists is equal) break.
        if (result.size() == selection.size())
            break;
    }

    return result;
}

void XMLDB::Database::insertList(
        const DB::FileName& fileName,
        const DB::ImageInfoList& list,
        bool after)
{
    DB::ImageInfoListIterator imageIt = _images.begin();
    for( ; imageIt != _images.end(); ++imageIt ) {
        if ( (*imageIt)->fileName() == fileName ) {
            break;
        }
    }
    // since insert() inserts before iterator increment when inserting AFTER image
    if ( after )
        imageIt++;
    for( DB::ImageInfoListConstIterator it = list.begin(); it != list.end(); ++it ) {
        // the call to insert() destroys the given iterator so use the new one after the call
        imageIt = _images.insert( imageIt, *it );
        // increment always to retain order of selected images
        imageIt++;
    }
    emit dirty();
}


bool XMLDB::Database::stack(const DB::FileNameList& items)
{
    unsigned int changed = 0;
    QSet<DB::StackID> stacks;
    QList<DB::ImageInfoPtr> images;
    unsigned int stackOrder = 1;

    Q_FOREACH(const DB::FileName& fileName, items) {
        DB::ImageInfoPtr imgInfo = fileName.info();
        Q_ASSERT( imgInfo );
        if ( imgInfo->isStacked() ) {
            stacks << imgInfo->stackId();
            stackOrder = qMax( stackOrder, imgInfo->stackOrder() + 1 );
        } else {
            images << imgInfo;
        }
    }

    if ( stacks.size() > 1 )
        return false; // images already in different stacks -> can't stack

    DB::StackID stackId = ( stacks.size() == 1 ) ? *(stacks.begin() ) : _nextStackId++;
    for ( QList<DB::ImageInfoPtr>::iterator it = images.begin();
          it != images.end();
          ++it, ++stackOrder ) {
        (*it)->setStackOrder( stackOrder );
        (*it)->setStackId( stackId );
        _stackMap[stackId].append((*it)->fileName());
        ++changed;
    }

    if ( changed )
        emit dirty();

    return changed;
}

void XMLDB::Database::unstack(const DB::FileNameList& items)
{
    Q_FOREACH(const DB::FileName& fileName, items) {
        DB::FileNameList allInStack = getStackFor(fileName);
        if (allInStack.size() <= 2) {
            // we're destroying stack here
            Q_FOREACH(const DB::FileName& stackFileName, allInStack) {
                DB::ImageInfoPtr imgInfo = stackFileName.info();
                Q_ASSERT( imgInfo );
                if ( imgInfo->isStacked() ) {
                    _stackMap.remove( imgInfo->stackId() );
                    imgInfo->setStackId( 0 );
                    imgInfo->setStackOrder( 0 );
                }
            }
        } else {
            DB::ImageInfoPtr imgInfo = fileName.info();
            Q_ASSERT( imgInfo );
            if ( imgInfo->isStacked() ) {
                _stackMap[imgInfo->stackId()].removeAll(fileName);
                imgInfo->setStackId( 0 );
                imgInfo->setStackOrder( 0 );
            }
        }
    }

    if (!items.isEmpty())
        emit dirty();
}

DB::FileNameList XMLDB::Database::getStackFor(const DB::FileName& referenceImg) const
{
    DB::ImageInfoPtr imageInfo = info( referenceImg );

    if ( !imageInfo || ! imageInfo->isStacked() )
        return DB::FileNameList();

    StackMap::iterator found = _stackMap.find(imageInfo->stackId());
    if ( found != _stackMap.end() )
        return found.value();

    // it wasn't in the cache -> rebuild it
    _stackMap.clear();
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        if ( (*it)->isStacked() ) {
            DB::StackID stackid = (*it)->stackId();
            _stackMap[stackid].append((*it)->fileName());
        }
    }

    found = _stackMap.find(imageInfo->stackId());
    if ( found != _stackMap.end() )
        return found.value();
    else
        return DB::FileNameList();
}

void XMLDB::Database::copyData(const DB::FileName &from, const DB::FileName &to)
{
    (*info(to)).merge(*info(from));
}


// During profiling of loading, I found that a significant amount of time was spent in QDateTime::fromString.
// Reviewing the code, I fount that it did a lot of extra checks we don't need (like checking if the string have
// timezone information (which they won't in KPA), this function is a replacement that is faster than the original.
QDateTime dateTimeFromString(const QString& str) {
    static QChar T = QChar::fromLatin1('T');

    if ( str[10] == T)
        return QDateTime(QDate::fromString(str.left(10), Qt::ISODate),QTime::fromString(str.mid(11),Qt::ISODate));

    else
        return QDateTime::fromString(str,Qt::ISODate);
}

DB::ImageInfoPtr XMLDB::Database::createImageInfo( const DB::FileName& fileName, ReaderPtr reader, Database* db )
{
    static QString _label_ = QString::fromUtf8("label");
    static QString _description_ = QString::fromUtf8("description");
    static QString _startDate_ = QString::fromUtf8("startDate");
    static QString _endDate_ = QString::fromUtf8("endDate");
    static QString _yearFrom_ = QString::fromUtf8("yearFrom");
    static QString _monthFrom_ = QString::fromUtf8("monthFrom");
    static QString _dayFrom_ = QString::fromUtf8("dayFrom");
    static QString _hourFrom_ = QString::fromUtf8("hourFrom");
    static QString _minuteFrom_ = QString::fromUtf8("minuteFrom");
    static QString _secondFrom_ = QString::fromUtf8("secondFrom");
    static QString _yearTo_ = QString::fromUtf8("yearTo");
    static QString _monthTo_ = QString::fromUtf8("monthTo");
    static QString _dayTo_ = QString::fromUtf8("dayTo");
    static QString _angle_ = QString::fromUtf8("angle");
    static QString _md5sum_ = QString::fromUtf8("md5sum");
    static QString _width_ = QString::fromUtf8("width");
    static QString _height_ = QString::fromUtf8("height");
    static QString _rating_ = QString::fromUtf8("rating");
    static QString _stackId_ = QString::fromUtf8("stackId");
    static QString _stackOrder_ = QString::fromUtf8("stackOrder");
    static QString _gpsPrec_ = QString::fromUtf8("gpsPrec");
    static QString _gpsLon_ = QString::fromUtf8("gpsLon");
    static QString _gpsLat_ = QString::fromUtf8("gpsLat");
    static QString _gpsAlt_ = QString::fromUtf8("gpsAlt");
    static QString _videoLength_ = QString::fromUtf8("videoLength");
    static QString _options_ = QString::fromUtf8("options");
    static QString _0_ = QString::fromUtf8("0");
    static QString _minus1_ = QString::fromUtf8("-1");
    static QString _MediaType_ = QString::fromUtf8("Media Type");
    static QString _Image_ = QString::fromUtf8("Image");
    static QString _Video_ = QString::fromUtf8("Video");

    QString label = reader->attribute(_label_);
    QString description;
    if ( reader->hasAttribute(_description_) )
        description = reader->attribute(_description_);

    DB::ImageDate date;
    if ( reader->hasAttribute(_startDate_) ) {
        QDateTime start;
        QDateTime end;

        QString str = reader->attribute(  _startDate_  );
        if ( !str.isEmpty() )
            start = dateTimeFromString( str );

        str = reader->attribute(  _endDate_  );
        if ( !str.isEmpty() )
            end = dateTimeFromString(str);
        date = DB::ImageDate( start, end );
    }
    else {
        int yearFrom = 0, monthFrom = 0,  dayFrom = 0, yearTo = 0, monthTo = 0,  dayTo = 0, hourFrom = -1, minuteFrom = -1, secondFrom = -1;

        yearFrom = reader->attribute( _yearFrom_, _0_ ).toInt();
        monthFrom = reader->attribute( _monthFrom_, _0_ ).toInt();
        dayFrom = reader->attribute( _dayFrom_, _0_ ).toInt();
        hourFrom = reader->attribute( _hourFrom_, _minus1_ ).toInt();
        minuteFrom = reader->attribute( _minuteFrom_, _minus1_ ).toInt();
        secondFrom = reader->attribute( _secondFrom_, _minus1_ ).toInt();

        yearTo = reader->attribute( _yearTo_, _0_ ).toInt();
        monthTo = reader->attribute( _monthTo_, _0_ ).toInt();
        dayTo = reader->attribute( _dayTo_, _0_ ).toInt();
        date = DB::ImageDate( yearFrom, monthFrom, dayFrom, yearTo, monthTo, dayTo, hourFrom, minuteFrom, secondFrom );
    }

    int angle = reader->attribute( _angle_, _0_).toInt();
    DB::MD5 md5sum(reader->attribute(  _md5sum_  ));

    _anyImageWithEmptySize |= !reader->hasAttribute(_width_);

    int w = reader->attribute(  _width_ , _minus1_ ).toInt();
    int h = reader->attribute(  _height_ , _minus1_ ).toInt();
    QSize size = QSize( w,h );

    DB::MediaType mediaType = Utilities::isVideo(fileName) ? DB::Video : DB::Image;

    short rating = reader->attribute( _rating_, _minus1_ ).toShort();
    DB::StackID stackId = reader->attribute( _stackId_, _0_ ).toULong();
    unsigned int stackOrder = reader->attribute( _stackOrder_, _0_ ).toULong();

    DB::ImageInfo* info = new DB::ImageInfo( fileName, label, description, date,
                                             angle, md5sum, size, mediaType, rating, stackId, stackOrder );

    static QString _defaultPrecision_ = QString::number(DB::GpsCoordinates::PrecisionDataForNull);
    int gpsPrecision = reader->attribute(
                _gpsPrec_,
                _defaultPrecision_).toInt();
    if ( gpsPrecision != DB::GpsCoordinates::PrecisionDataForNull )
        info->setGeoPosition(
                    DB::GpsCoordinates(
                        reader->attribute( _gpsLon_ ).toDouble(),
                        reader->attribute( _gpsLat_ ).toDouble(),
                        reader->attribute( _gpsAlt_ ).toDouble(),
                        gpsPrecision));

    if ( reader->hasAttribute(_videoLength_))
        info->setVideoLength(reader->attribute(_videoLength_).toInt());

    DB::ImageInfoPtr result(info);

    possibleLoadCompressedCategories( reader, result, db );

    while( reader->readNextStartOrStopElement(_options_)) {
        readOptions( result, reader );
    }

    info->addCategoryInfo( _MediaType_,
                           info->mediaType() == DB::Image ? _Image_ : _Video_ );

    return result;
}

void XMLDB::Database::readOptions( DB::ImageInfoPtr info, ReaderPtr reader )
{
    static QString _name_ = QString::fromUtf8("name");
    static QString _value_ = QString::fromUtf8("value");
    static QString _option_ = QString::fromUtf8("option");

    while (reader->readNextStartOrStopElement(_option_)) {
        QString name = FileReader::unescape( reader->attribute(_name_) );

        if ( !name.isNull() )  {
            // Read values
            while (reader->readNextStartOrStopElement(_value_)) {
                QString value = reader->attribute(_value_);
                if ( !value.isNull() )  {
                    info->addCategoryInfo( name, value );
                }
                reader->readEndElement();
            }
        }
    }
}



void XMLDB::Database::possibleLoadCompressedCategories( ReaderPtr reader, DB::ImageInfoPtr info, Database* db )
{
    if ( db == 0 )
        return;

    QList<DB::CategoryPtr> categoryList = db->_categoryCollection.categories();
    for( QList<DB::CategoryPtr>::Iterator categoryIt = categoryList.begin(); categoryIt != categoryList.end(); ++categoryIt ) {
        QString categoryName = (*categoryIt)->name();
        QString str = reader->attribute( FileWriter::escape( categoryName ) );
        if ( !str.isEmpty() ) {
            QStringList list = str.split(QString::fromLatin1( "," ), QString::SkipEmptyParts );
            for( QStringList::Iterator listIt = list.begin(); listIt != list.end(); ++listIt ) {
                int id = (*listIt).toInt();
                QString name = static_cast<XMLCategory*>((*categoryIt).data())->nameForId(id);
                info->addCategoryInfo( categoryName, name );
            }
        }
    }
}


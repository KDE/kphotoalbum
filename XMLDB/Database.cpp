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
#include "DB/IdList.h"
#include "DB/Id.h"
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
#include <Q3ValueList>
#ifdef HAVE_EXIV2
#   include "Exif/Database.h"
#endif

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

bool XMLDB::Database::operator==(const DB::ImageDB& other) const
{
    const XMLDB::Database* xmlOther =
        dynamic_cast<const XMLDB::Database*>(&other);
    if (!xmlOther)
        return false;
    return Utilities::areSameFile(_fileName, xmlOther->_fileName);
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
    Q3Dict<void> alreadyMatched = info.findAlreadyMatched( category );

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
                if ( !alreadyMatched[*it2] ) // We do not want to match "Jesper & Jesper"
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

void XMLDB::Database::addToBlockList(const DB::IdList& list)
{
    Q_FOREACH(DB::ImageInfoPtr inf, list.fetchInfos()) {
        _blockList << inf->fileName( DB::RelativeToImageRoot );
    }
    deleteList( list );
}

void XMLDB::Database::deleteList(const DB::IdList& list)
{
    Q_FOREACH(DB::Id id, list) {
        DB::ImageInfoPtr inf = id.fetchInfo();
        StackMap::iterator found = _stackMap.find(inf->stackId());
        if ( inf->isStacked() && found != _stackMap.end() ) {
            const DB::IdList& origCache = found.value();
            DB::IdList newCache;
            Q_FOREACH(DB::Id cacheId, origCache) {
                if (id != cacheId)
                    newCache.append(cacheId);
            }
            if (newCache.size() <= 1) {
                // we're destroying a stack
                Q_FOREACH(DB::ImageInfoPtr cacheInf, newCache.fetchInfos()) {
                    cacheInf->setStackId(0);
                    cacheInf->setStackOrder(0);
                }
                _stackMap.remove( inf->stackId() );
            } else {
                _stackMap.insert(inf->stackId(), newCache);
            }
        }
#ifdef HAVE_EXIV2
        Exif::Database::instance()->remove( inf->fileName( DB::AbsolutePath) );
#endif
        _idMapper.remove( inf->fileName(DB::RelativeToImageRoot) );
        _images.remove( inf );
    }
    emit totalChanged( _images.count() );
    emit imagesDeleted( list );
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

    Q_FOREACH( const DB::ImageInfoPtr& info, images ) {
        _idMapper.add( info->fileName(DB::RelativeToImageRoot) );
    }

    emit totalChanged( _images.count() );
    emit dirty();
}

void XMLDB::Database::renameImage( DB::ImageInfoPtr info, const QString& newName )
{
    info->delaySavingChanges(false);
    _idMapper.remove( info->fileName(DB::RelativeToImageRoot) );
    info->setFileName( newName );
    _idMapper.add( info->fileName(DB::RelativeToImageRoot) );
}

DB::ImageInfoPtr XMLDB::Database::info( const QString& fileName, DB::PathType type ) const
{
    static QMap<QString, DB::ImageInfoPtr > fileMap;

    QString name = fileName;
    if ( type == DB::RelativeToImageRoot )
        name = Settings::SettingsData::instance()->imageDirectory() + fileName;

    if ( fileMap.contains( name ) )
        return fileMap[ name ];
    else {
        fileMap.clear();
        for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
            fileMap.insert( (*it)->fileName(DB::AbsolutePath), *it );
        }
        if ( fileMap.contains( name ) )
            return fileMap[ name ];
    }
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

bool XMLDB::Database::isBlocking( const QString& fileName )
{
    return _blockList.contains( fileName );
}


DB::IdList XMLDB::Database::images()
{
    QList<DB::RawId> result;
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        result.append( _idMapper[(*it)->fileName( DB::RelativeToImageRoot )]);
    }
    return DB::IdList(result);
}

DB::IdList XMLDB::Database::search(
    const DB::ImageSearchInfo& info,
    bool requireOnDisk) const
{
    return searchPrivate( info, requireOnDisk, true );
}

DB::IdList XMLDB::Database::searchPrivate(
    const DB::ImageSearchInfo& info,
    bool requireOnDisk,
    bool onlyItemsMatchingRange) const
{
    // When searching for images counts for the datebar, we want matches outside the range too.
    // When searching for images for the thumbnail view, we only want matches inside the range.
    QList<DB::RawId> result;
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && ( !onlyItemsMatchingRange || rangeInclude( *it ));
        match &= !requireOnDisk || DB::ImageInfo::imageOnDisk( (*it)->fileName(DB::AbsolutePath) );

        if (match)
            result.append(_idMapper[(*it)->fileName( DB::RelativeToImageRoot )]);
    }
    return DB::IdList(result);
}

void XMLDB::Database::sortAndMergeBackIn(const DB::IdList& idList)
{
    DB::ImageInfoList infoList;
    infoList += idList.fetchInfos();
    _images.sortAndMergeBackIn(infoList);
}

DB::CategoryCollection* XMLDB::Database::categoryCollection()
{
    return &_categoryCollection;
}

KSharedPtr<DB::ImageDateCollection> XMLDB::Database::rangeCollection()
{
    return KSharedPtr<DB::ImageDateCollection>(
        new XMLImageDateCollection( searchPrivate( Browser::BrowserWidget::instance()->currentContext(), false, false ) ) );
}

void XMLDB::Database::reorder(
    const DB::Id& item,
    const DB::IdList& selection,
    bool after)
{
    Q_ASSERT(!item.isNull());
    DB::ImageInfoList list = takeImagesFromSelection( selection );
    insertList( item, list, after );
}

// Remove all the images from the database that match the given selection and
// return that sublist.
// This returns the selected and erased images in the order in which they appear
// in the image list itself.
DB::ImageInfoList XMLDB::Database::takeImagesFromSelection(const DB::IdList& selection)
{
    DB::ImageInfoList result;
    if (selection.isEmpty())
        return result;

    // iterate over all images (expensive!!) TODO: improve?
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); /**/ ) {
        QString imagefile = (*it)->fileName(DB::AbsolutePath);
        DB::IdList::ConstIterator si = selection.begin();
        // for each image, iterate over selection, break on match
        for ( /**/; si != selection.end(); ++si ) {
            QString file = (*si).fetchInfo()->fileName(DB::AbsolutePath);
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
    const DB::Id& id,
    const DB::ImageInfoList& list,
    bool after)
{
    QString fileName = id.fetchInfo()->fileName(DB::AbsolutePath);

    DB::ImageInfoListIterator imageIt = _images.begin();
    for( ; imageIt != _images.end(); ++imageIt ) {
        if ( (*imageIt)->fileName(DB::AbsolutePath) == fileName ) {
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


bool XMLDB::Database::stack(const DB::IdList& items)
{
    unsigned int changed = 0;
    QSet<DB::StackID> stacks;
    QList<DB::ImageInfoPtr> images;
    unsigned int stackOrder = 1;

    Q_FOREACH(DB::ImageInfoPtr imgInfo, items.fetchInfos()) {
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
        _stackMap[stackId].append(ID_FOR_FILE((*it)->fileName(DB::AbsolutePath)));
        ++changed;
    }

    if ( changed )
        emit dirty();

    return changed;
}

void XMLDB::Database::unstack(const DB::IdList& items)
{
    Q_FOREACH(DB::Id id, items) {
        DB::IdList allInStack = getStackFor(id);
        if (allInStack.size() <= 2) {
            // we're destroying stack here
            Q_FOREACH(DB::ImageInfoPtr imgInfo, allInStack.fetchInfos()) {
                Q_ASSERT( imgInfo );
                if ( imgInfo->isStacked() ) {
                    _stackMap.remove( imgInfo->stackId() );
                    imgInfo->setStackId( 0 );
                    imgInfo->setStackOrder( 0 );
                }
            }
        } else {
            DB::ImageInfoPtr imgInfo = id.fetchInfo();
            Q_ASSERT( imgInfo );
            if ( imgInfo->isStacked() ) {
                _stackMap[imgInfo->stackId()].removeAll(id);
                imgInfo->setStackId( 0 );
                imgInfo->setStackOrder( 0 );
            }
        }
    }

    if (!items.isEmpty())
        emit dirty();
}

DB::IdList XMLDB::Database::getStackFor(const DB::Id& referenceImg) const
{
    DB::ImageInfoPtr imageInfo = info( referenceImg );

    if ( !imageInfo || ! imageInfo->isStacked() )
        return DB::IdList();

    StackMap::iterator found = _stackMap.find(imageInfo->stackId());
    if ( found != _stackMap.end() )
        return found.value();

    // it wasn't in the cache -> rebuild it
    _stackMap.clear();
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        if ( (*it)->isStacked() ) {
            DB::StackID stackid = (*it)->stackId();
            _stackMap[stackid].append(ID_FOR_FILE((*it)->fileName(DB::AbsolutePath))); // will need to be sorted later
        }
    }

#ifdef KDAB_TEMPORARILY_REMOVED  // TODO(hzeller)/QWERTY: won't work with the limited iterator impl. of IdList.
    StackSortHelper sortHelper( this );
    for ( QMap<DB::StackID,QStringList>::iterator it = _stackMap.begin(); it != _stackMap.end(); ++it ) {
        qSort( it->begin(), it->end(), sortHelper );
    }
#endif

    found = _stackMap.find(imageInfo->stackId());
    if ( found != _stackMap.end() )
        return found.value();
    else
        return DB::IdList();
}

XMLDB::Database::StackSortHelper::StackSortHelper( const Database* const db ): _db(db)
{
}

int XMLDB::Database::StackSortHelper::operator()( const QString& fileA, const QString& fileB ) const
{
    DB::ImageInfoPtr a = _db->info( fileA, DB::AbsolutePath );
    DB::ImageInfoPtr b = _db->info( fileB, DB::AbsolutePath );
    if ( !a && !b )
        return -1;
    return ( a->stackId() == b->stackId() ) && ( a->stackOrder() < b->stackOrder() );
}

DB::ImageInfoPtr XMLDB::Database::createImageInfo( const QString& fileName, const QDomElement& elm, Database* db )
{
    QString label = elm.attribute( QString::fromLatin1("label") );
    QString description;
    if ( elm.hasAttribute( QString::fromLatin1( "description" ) ) )
         description = elm.attribute( QString::fromLatin1("description") );

    DB::ImageDate date;
    if ( elm.hasAttribute( QString::fromLatin1( "startDate" ) ) ) {
        QDateTime start;
        QDateTime end;

        QString str = elm.attribute( QString::fromLatin1( "startDate" ) );
        if ( !str.isEmpty() )
            start = QDateTime::fromString( str, Qt::ISODate );

        str = elm.attribute( QString::fromLatin1( "endDate" ) );
        if ( !str.isEmpty() )
            end = QDateTime::fromString( str, Qt::ISODate );
        date = DB::ImageDate( start, end );
    }
    else {
        int yearFrom = 0, monthFrom = 0,  dayFrom = 0, yearTo = 0, monthTo = 0,  dayTo = 0, hourFrom = -1, minuteFrom = -1, secondFrom = -1;

        yearFrom = elm.attribute( QString::fromLatin1("yearFrom"), QString::number( yearFrom) ).toInt();
        monthFrom = elm.attribute( QString::fromLatin1("monthFrom"), QString::number(monthFrom) ).toInt();
        dayFrom = elm.attribute( QString::fromLatin1("dayFrom"), QString::number(dayFrom) ).toInt();
        hourFrom = elm.attribute( QString::fromLatin1("hourFrom"), QString::number(hourFrom) ).toInt();
        minuteFrom = elm.attribute( QString::fromLatin1("minuteFrom"), QString::number(minuteFrom) ).toInt();
        secondFrom = elm.attribute( QString::fromLatin1("secondFrom"), QString::number(secondFrom) ).toInt();

        yearTo = elm.attribute( QString::fromLatin1("yearTo"), QString::number(yearTo) ).toInt();
        monthTo = elm.attribute( QString::fromLatin1("monthTo"), QString::number(monthTo) ).toInt();
        dayTo = elm.attribute( QString::fromLatin1("dayTo"), QString::number(dayTo) ).toInt();
        date = DB::ImageDate( yearFrom, monthFrom, dayFrom, yearTo, monthTo, dayTo, hourFrom, minuteFrom, secondFrom );
    }

    int angle = elm.attribute( QString::fromLatin1("angle"), QString::fromLatin1("0") ).toInt();
    DB::MD5 md5sum(elm.attribute( QString::fromLatin1( "md5sum" ) ));

    _anyImageWithEmptySize |= !elm.hasAttribute( QString::fromLatin1( "width" ) );

    int w = elm.attribute( QString::fromLatin1( "width" ), QString::fromLatin1( "-1" ) ).toInt();
    int h = elm.attribute( QString::fromLatin1( "height" ), QString::fromLatin1( "-1" ) ).toInt();
    QSize size = QSize( w,h );

    DB::MediaType mediaType = Utilities::isVideo(fileName) ? DB::Video : DB::Image;

    short rating = elm.attribute( QString::fromLatin1("rating"), QString::fromLatin1("-1") ).toShort();
    DB::StackID stackId = elm.attribute( QString::fromLatin1("stackId"), QString::fromLatin1("0") ).toULong();
    unsigned int stackOrder = elm.attribute( QString::fromLatin1("stackOrder"), QString::fromLatin1("0") ).toULong();

    DB::ImageInfo* info = new DB::ImageInfo( fileName, label, description, date,
            angle, md5sum, size, mediaType, rating, stackId, stackOrder );

    int gpsPrecision = elm.attribute(
        QLatin1String("gpsPrec"),
        QString::number(DB::GpsCoordinates::PrecisionDataForNull)).toInt();
    if ( gpsPrecision != DB::GpsCoordinates::PrecisionDataForNull )
        info->setGeoPosition(
            DB::GpsCoordinates(
                elm.attribute( QLatin1String("gpsLon") ).toDouble(),
                elm.attribute( QLatin1String("gpsLat") ).toDouble(),
                elm.attribute( QLatin1String("gpsAlt") ).toDouble(),
                gpsPrecision));

    DB::ImageInfoPtr result(info);
    for ( QDomNode child = elm.firstChild(); !child.isNull(); child = child.nextSibling() ) {
        if ( child.isElement() ) {
            QDomElement childElm = child.toElement();
            if ( childElm.tagName() == QString::fromLatin1( "categories" ) || childElm.tagName() == QString::fromLatin1( "options" ) ) {
                // options is for KimDaBa 2.1 compatibility
                readOptions( result, childElm );
            }
            else if ( childElm.tagName() == QString::fromLatin1( "drawings" ) ) {
                // Ignore - KPhotoAlbum 3.0 and older version had drawings, that is not supported anymore
            }
            else {
                KMessageBox::error( 0, i18n("<p>Unknown tag %1, while reading configuration file.</p>"
                                            "<p>Expected one of: Options, Drawings</p>", childElm.tagName() ) );
            }
        }
    }

    possibleLoadCompressedCategories( elm, result, db );

    info->addCategoryInfo( QString::fromLatin1( "Media Type" ),
                     info->mediaType() == DB::Image ? QString::fromLatin1( "Image" ) : QString::fromLatin1( "Video" ) );

    return result;
}

void XMLDB::Database::readOptions( DB::ImageInfoPtr info, QDomElement elm )
{
    // options is for KimDaBa 2.1 compatibility
    Q_ASSERT( elm.tagName() == QString::fromLatin1( "categories" ) || elm.tagName() == QString::fromLatin1( "options" ) );

    for ( QDomNode nodeOption = elm.firstChild(); !nodeOption.isNull(); nodeOption = nodeOption.nextSibling() )  {

        if ( nodeOption.isElement() )  {
            QDomElement elmOption = nodeOption.toElement();
            // option is for KimDaBa 2.1 compatibility
            Q_ASSERT( elmOption.tagName() == QString::fromLatin1("category") || elmOption.tagName() == QString::fromLatin1("option") );
            QString name = FileReader::unescape( elmOption.attribute( QString::fromLatin1("name") ) );

            if ( !name.isNull() )  {
                // Read values
                for ( QDomNode nodeValue = elmOption.firstChild(); !nodeValue.isNull();
                      nodeValue = nodeValue.nextSibling() ) {
                    if ( nodeValue.isElement() ) {
                        QDomElement elmValue = nodeValue.toElement();
                        Q_ASSERT( elmValue.tagName() == QString::fromLatin1("value") );
                        QString value = elmValue.attribute( QString::fromLatin1("value") );
                        if ( !value.isNull() )  {
                            info->addCategoryInfo( name, value );
                        }
                    }
                }
            }
        }
    }
}

void XMLDB::Database::possibleLoadCompressedCategories( const QDomElement& elm, DB::ImageInfoPtr info, Database* db )
{
    if ( db == 0 )
        return;

    Q3ValueList<DB::CategoryPtr> categoryList = db->_categoryCollection.categories();
    for( Q3ValueList<DB::CategoryPtr>::Iterator categoryIt = categoryList.begin(); categoryIt != categoryList.end(); ++categoryIt ) {
        QString categoryName = (*categoryIt)->name();
        QString str = elm.attribute( FileWriter::escape( categoryName ) );
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

// PENDING(blackie) THIS NEEDS TO GO AWAY //QWERTY
QStringList XMLDB::Database::CONVERT(const DB::IdList& items)
{
    QStringList result;
    Q_FOREACH(DB::Id id, items) {
        result << Utilities::absoluteImageFileName(_idMapper[id.rawId()]);
    }
    return result;
}

DB::Id XMLDB::Database::ID_FOR_FILE( const QString& filename) const {
    return DB::Id::createContextless(_idMapper[ Utilities::imageFileNameToRelative(filename)]);
}

DB::ImageInfoPtr XMLDB::Database::info( const DB::Id& id) const
{
    if (id.isNull())
        return DB::ImageInfoPtr(NULL);
    return info( _idMapper[id.rawId()],DB::RelativeToImageRoot);
}

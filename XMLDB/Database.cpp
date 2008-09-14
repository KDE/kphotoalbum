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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Database.h"
#include "DB/Result.h"
#include "DB/ResultId.h"
#include "Settings/SettingsData.h"
#include <kmessagebox.h>
#include <klocale.h>
#include "Utilities/Util.h"
#include "DB/GroupCounter.h"
#include "Browser/BrowserWidget.h"
#include "DB/ImageInfo.h"
#include "DB/ImageInfoPtr.h"
#include "DB/CategoryCollection.h"
#include "DB/ResultId.h"
#include "Database.moc"
#include "XMLCategory.h"
#include <ksharedptr.h>
#include "XMLImageDateCollection.h"
#include "FileReader.h"
#include "FileWriter.h"
#include "MainWindow/DirtyIndicator.h"
//Added by qt3to4:
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
QMap<QString,uint> XMLDB::Database::classify( const DB::ImageSearchInfo& info, const QString &group, DB::MediaType typemask )
{
    QMap<QString, uint> map;
    DB::GroupCounter counter( group );
    Q3Dict<void> alreadyMatched = info.findAlreadyMatched( group );

    DB::ImageSearchInfo noMatchInfo = info;
    QString currentMatchTxt = noMatchInfo.option( group );
    if ( currentMatchTxt.isEmpty() )
        noMatchInfo.setOption( group, DB::ImageDB::NONE() );
    else
        noMatchInfo.setOption( group, QString::fromLatin1( "%1 & %2" ).arg(currentMatchTxt).arg(DB::ImageDB::NONE()) );

    // Iterate through the whole database of images.
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        bool match = ( (*it)->mediaType() & typemask ) && !(*it)->isLocked() && info.match( *it ) && rangeInclude( *it );
        if ( match ) { // If the given image is currently matched.

            // Now iterate through all the categories the current image
            // contains, and increase them in the map mapping from category
            // to count.
            StringSet items = (*it)->itemsOfCategory(group);
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

void XMLDB::Database::addToBlockList( const DB::ConstResultPtr& list )
{
    for( DB::Result::ConstIterator it = list->begin(); it != list->end(); ++it ) {
        DB::ImageInfoPtr inf= info(*it );
        _blockList << inf->fileName( DB::RelativeToImageRoot );
        _idMapper.remove( inf->fileName( DB::RelativeToImageRoot ) );
        _images.remove( inf );
    }
    emit totalChanged( _images.count() );
}

void XMLDB::Database::deleteList( const DB::ConstResultPtr& list )
{
    for( DB::Result::ConstIterator it = list->begin(); it != list->end(); ++it ) {
        DB::ImageInfoPtr inf= info(*it );
        StackMap::iterator found = _stackMap.find(inf->stackId());
        if ( inf->isStacked() && found != _stackMap.end() ) {
            const DB::ResultPtr& origCache = found.value();
            DB::ResultPtr newCache = new DB::Result();
            for ( DB::Result::const_iterator cacheIt = origCache->begin(); cacheIt != origCache->end(); ++cacheIt ) {
                if ( *it != *cacheIt )
                    newCache->append(*cacheIt);
            }
            if ( newCache->size() <= 1 ) {
                // we're destroying a stack
                for ( DB::Result::const_iterator it = newCache->begin(); it != newCache->end(); ++it ) {
                    DB::ImageInfoPtr inf = info( *it );
                    inf->setStackId( 0 );
                    inf->setStackOrder( 0 );
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
    MainWindow::DirtyIndicator::markDirty();
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


DB::ConstResultPtr XMLDB::Database::images()
{
    QList<int> result;
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); ++it ) {
        result.append( _idMapper[(*it)->fileName( DB::RelativeToImageRoot )]);
    }
    return DB::ResultPtr( new DB::Result(result) );
}

DB::ConstResultPtr XMLDB::Database::search( const DB::ImageSearchInfo& info, bool requireOnDisk ) const
{
    return searchPrivate( info, requireOnDisk, true );
}

DB::ConstResultPtr XMLDB::Database::searchPrivate( const DB::ImageSearchInfo& info, bool requireOnDisk, bool onlyItemsMatchingRange ) const
{
    // When searching for images counts for the datebar, we want matches outside the range too.
    // When searching for images for the thumbnail view, we only want matches inside the range.
    QList<int> result;
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        bool match = !(*it)->isLocked() && info.match( *it ) && ( !onlyItemsMatchingRange || rangeInclude( *it ));
        match &= !requireOnDisk || DB::ImageInfo::imageOnDisk( (*it)->fileName(DB::AbsolutePath) );

        if (match)
            result.append(_idMapper[(*it)->fileName( DB::RelativeToImageRoot )]);
    }
    return DB::ResultPtr( new DB::Result(result) );
}

void XMLDB::Database::sortAndMergeBackIn( const DB::ConstResultPtr& idList )
{
    DB::ImageInfoList list;

    for( DB::Result::ConstIterator it = idList->begin(); it != idList->end(); ++it ) {
        list.append( (*it).fetchInfo() );
    }
    _images.sortAndMergeBackIn( list );
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

void XMLDB::Database::reorder( const DB::ResultId& item, const DB::ConstResultPtr& selection, bool after) {
    Q_ASSERT(!item.isNull());
    Q_ASSERT(!selection.isNull());
    DB::ImageInfoList list = takeImagesFromSelection( selection );
    insertList( item, list, after );
}

// Remove all the images from the database that match the given selection and
// return that sublist.
// Note: The selection is known to be sorted wrt the order in the image list,
// i.e. it is a common subsequence.
DB::ImageInfoList XMLDB::Database::takeImagesFromSelection( const DB::ConstResultPtr& selection )
{  
    DB::ImageInfoList result;
    if (selection->isEmpty())
        return result;

    DB::Result::ConstIterator subsequenceIt = selection->begin();
    QString file = (*subsequenceIt).fetchInfo()->fileName(DB::AbsolutePath);
 
    for( DB::ImageInfoListIterator it = _images.begin(); it != _images.end(); /**/ ) {
        if ( (*it)->fileName(DB::AbsolutePath) == file ) {
            result << *it;
            it = _images.erase(it);
            ++subsequenceIt;
            if (subsequenceIt == selection->end())
                break;
            file = (*subsequenceIt).fetchInfo()->fileName(DB::AbsolutePath);
        } else {
            ++it;
        }
    }
    Q_ASSERT( subsequenceIt == selection->end() );  // if not, selection was not a subsequence

    return result;
}

DB::ConstResultPtr XMLDB::Database::insertList( const DB::ResultId& id, const DB::ImageInfoList& list, bool after )
{
    DB::ResultPtr result = new DB::Result();
    QString fileName = id.fetchInfo()->fileName(DB::AbsolutePath);

    DB::ImageInfoListIterator imageIt = _images.begin();
    for( ; imageIt != _images.end(); ++imageIt ) {
        if ( (*imageIt)->fileName(DB::AbsolutePath) == fileName ) {
            break;
        }
    }

    if ( after )
        imageIt++;
    for( DB::ImageInfoListConstIterator it = list.begin(); it != list.end(); ++it ) {
        _images.insert( imageIt, *it );
        result->append( ID_FOR_FILE((*it)->fileName(DB::AbsolutePath)));
    }
    MainWindow::DirtyIndicator::markDirty();

    return result;
}


void XMLDB::Database::cutToClipboard( const QStringList& )
{
#ifdef KDAB_TEMPORARILY_REMOVED
    _clipboard = takeImagesFromSelection( selection );
#endif
}

QStringList XMLDB::Database::pasteFromCliboard( const QString& )
{
#ifdef KDAB_TEMPORARILY_REMOVED
    QStringList result = insertList( afterFile, _clipboard, true );
    _clipboard.clear();
#else
    return QStringList();
#endif
}

bool XMLDB::Database::isClipboardEmpty()
{
    return _clipboard.isEmpty();
}

bool XMLDB::Database::stack( const DB::ConstResultPtr& items )
{
    unsigned int changed = 0;
    QSet<DB::StackID> stacks;
    QList<DB::ImageInfoPtr> images;
    unsigned int stackOrder = 1;

    for ( DB::Result::const_iterator it = items->begin(); it != items->end(); ++it ) {
        DB::ImageInfoPtr imgInfo = info( *it );
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
            ++it, stackOrder++ ) {
        (*it)->setStackOrder( stackOrder );
        (*it)->setStackId( stackId );
        StackMap::iterator found = _stackMap.find(stackId);
        if (found == _stackMap.end()) {
            found = _stackMap.insert(stackId, new DB::Result());
        }
        found.value()->append( ID_FOR_FILE((*it)->fileName(DB::AbsolutePath)) );
        ++changed;
    }

    if ( changed )
        MainWindow::DirtyIndicator::markDirty();

    return changed;
}

void XMLDB::Database::unstack( const DB::ConstResultPtr& items )
{
    for ( DB::Result::const_iterator it = items->begin(); it != items->end(); ++it ) {
        DB::ConstResultPtr allInStack = getStackFor( *it );
        if ( allInStack->size() <= 2 ) {
            // we're destroying stack here
            for ( DB::Result::const_iterator allIt = allInStack->begin();
                    allIt != allInStack->end(); ++allIt ) {
                DB::ImageInfoPtr imgInfo = info( *allIt );
                Q_ASSERT( imgInfo );
                if ( imgInfo->isStacked() ) {
                    _stackMap.remove( imgInfo->stackId() );
                    imgInfo->setStackId( 0 );
                    imgInfo->setStackOrder( 0 );
                }
            }
        } else {
            DB::ImageInfoPtr imgInfo = info( *it );
            Q_ASSERT( imgInfo );
            if ( imgInfo->isStacked() ) {
                DB::ResultPtr newCacheContents = new DB::Result();
                StackMap::iterator found = _stackMap.find(imgInfo->stackId());
                Q_ASSERT(found != _stackMap.end());
                const DB::ResultPtr& oldCache = found.value();
                for ( DB::Result::const_iterator it = oldCache->begin(); it != oldCache->end(); ++it ) {
                    if ( *it != ID_FOR_FILE(imgInfo->fileName(DB::AbsolutePath)) )
                        newCacheContents->append(*it);
                }
                _stackMap.insert(imgInfo->stackId(), newCacheContents);
                imgInfo->setStackId( 0 );
                imgInfo->setStackOrder( 0 );
            }
        }
    }

    if ( ! items->isEmpty() )
        MainWindow::DirtyIndicator::markDirty();
}

DB::ConstResultPtr XMLDB::Database::getStackFor( const DB::ResultId& referenceImg ) const
{
    DB::ImageInfoPtr imageInfo = info( referenceImg );

    if ( !imageInfo || ! imageInfo->isStacked() )
        return new DB::Result();

    StackMap::iterator found = _stackMap.find(imageInfo->stackId());
    if ( found != _stackMap.end() )
        return found.value();

    // it wasn't in the cache -> rebuild it
    _stackMap.clear();
    for( DB::ImageInfoListConstIterator it = _images.constBegin(); it != _images.constEnd(); ++it ) {
        if ( (*it)->isStacked() ) {
            DB::StackID stackid = (*it)->stackId();
            if (!_stackMap.contains(stackid)) {
                _stackMap.insert(stackid, new DB::Result());
            }
            StackMap::iterator found = _stackMap.find(stackid);
            Q_ASSERT(found != _stackMap.end());
            found.value()->append( ID_FOR_FILE((*it)->fileName(DB::AbsolutePath)) ); // will need to be sorted later
        }
    }

#ifdef KDAB_TEMPORARILY_REMOVED  // TODO(hzeller)/QWERTY: won't work with the limited iterator impl. of Result.
    StackSortHelper sortHelper( this );
    for ( QMap<DB::StackID,QStringList>::iterator it = _stackMap.begin(); it != _stackMap.end(); ++it ) {
        qSort( it->begin(), it->end(), sortHelper );
    }
#endif

    found = _stackMap.find(imageInfo->stackId());
    if ( found != _stackMap.end() )
        return found.value();
    else
        return new DB::Result();
}

XMLDB::Database::StackSortHelper::StackSortHelper( const Database* const db ): _db(db)
{
}

int XMLDB::Database::StackSortHelper::operator()( const QString& fileA, const QString& fileB ) const
{
    DB::ImageInfoPtr a = _db->info( fileA, DB::AbsolutePath );
    DB::ImageInfoPtr b = _db->info( fileB, DB::AbsolutePath );
    Q_ASSERT( a );
    Q_ASSERT( b );
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

    short rating = elm.attribute( QString::fromLatin1("rating"), "-1" ).toShort();
    DB::StackID stackId = elm.attribute( QString::fromLatin1("stackId"), "0" ).toULong();
    unsigned int stackOrder = elm.attribute( QString::fromLatin1("stackOrder"), "0" ).toULong();

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
                                            "<p>Expected one of: Options, Drawings</p>" ).arg( childElm.tagName() ) );
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
QStringList XMLDB::Database::CONVERT( const DB::ConstResultPtr& items )
{
    QStringList result;
    for ( int i = 0; i < items->size(); ++i ) {
        result << Utilities::absoluteImageFileName(_idMapper[items->at(i).fileId()]);
    }
    return result;
}

DB::ResultId XMLDB::Database::ID_FOR_FILE( const QString& filename) const {
    return DB::ResultId::createContextless(_idMapper[ Utilities::imageFileNameToRelative(filename)]);
}

DB::ImageInfoPtr XMLDB::Database::info( const DB::ResultId& id) const
{
    if (id.isNull())
        return DB::ImageInfoPtr(NULL);
    return info( _idMapper[id.fileId()],DB::RelativeToImageRoot);
}

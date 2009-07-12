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

#ifndef IMAGEDB_H
#define IMAGEDB_H

#include "DB/ImageInfoPtr.h"
#include "DB/ImageInfoList.h"
#include "DB/MediaCount.h"

#include <config-kpa-sqldb.h>
#ifdef SQLDB_SUPPORT
namespace SQLDB { class DatabaseAddress; }
#endif

class QProgressBar;

namespace DB
{

class CategoryCollection;
class Category;
class MD5Map;
class MemberMap;
class ImageDateCollection;
class Result;
class ResultId;
class ImageSearchInfo;

class ImageDB  :public QObject {
    Q_OBJECT

public:
    static ImageDB* instance();
    static void setupXMLDB( const QString& configFile );
#ifdef SQLDB_SUPPORT
    static void setupSQLDB( const SQLDB::DatabaseAddress& address );
#endif
    static void deleteInstance();

    void convertBackend(ImageDB* newBackend, QProgressBar* progressBar);
    virtual bool operator==(const ImageDB& other) const = 0;
    bool operator!=(const ImageDB& other) const { return !operator==(other); }
    StringSet imagesWithMD5Changed();

public slots:
    void setDateRange( const ImageDate&, bool includeFuzzyCounts );
    void clearDateRange();
    virtual void slotRescan();
    void slotRecalcCheckSums(const DB::Result& selection);
    virtual MediaCount count( const ImageSearchInfo& info );
    virtual void slotReread( const QStringList& list, DB::ExifMode mode);

protected:
    ImageDate _selectionRange;
    bool _includeFuzzyCounts;
    ImageInfoList _clipboard;

private:
    static void connectSlots();
    static ImageDB* _instance;

protected:
    ImageDB();

public:
    static QString NONE();
    DB::Result currentScope(bool requireOnDisk) const;

    virtual DB::ResultId findFirstItemInRange(
        const Result& images,
        const ImageDate& range,
        bool includeRanges) const;

public: // Methods that must be overriden
    virtual uint totalCount() const = 0;
    virtual DB::Result search(const ImageSearchInfo&, bool requireOnDisk=false) const = 0;

    virtual void renameCategory( const QString& oldName, const QString newName ) = 0;

    virtual QMap<QString,uint> classify( const ImageSearchInfo& info, const QString & category, MediaType typemask ) = 0;
    virtual Result images() = 0; // PENDING(blackie) TO BE REPLACED WITH URL's
    virtual void addImages( const ImageInfoList& images ) = 0;
    /** @short Update file name stored in the DB */
    virtual void renameImage( const ImageInfoPtr info, const QString& newName ) = 0;

    virtual void addToBlockList(const DB::Result& list) = 0;
    virtual bool isBlocking( const QString& fileName ) = 0;
    virtual void deleteList(const DB::Result& list) = 0;
    virtual ImageInfoPtr info( const QString& fileName, DB::PathType ) const = 0; //QWERTY DIE
    virtual MemberMap& memberMap() = 0;
    virtual void save( const QString& fileName, bool isAutoSave ) = 0;
    virtual MD5Map* md5Map() = 0;
    virtual void sortAndMergeBackIn(const DB::Result& idlist) = 0;

    virtual CategoryCollection* categoryCollection() = 0;
    virtual KSharedPtr<ImageDateCollection> rangeCollection() = 0;

    /**
     * Reorder the items in the database by placing all the items given in
     * cutList directly before or after the given item.
     * If the parameter "after" determines where to place it.
     */
    virtual void reorder(const DB::ResultId& item, const DB::Result& cutList, bool after) = 0;

    /**
     * temporary method to convert a DB::Result back to the usual
     * list of absolute filenames. This should not be necessary anymore after
     * the refactoring to use DB::Result everywhere
     */
    virtual QStringList CONVERT(const DB::Result&) = 0; //QWERTY DIE

    /**
     * there are some cases in which we have a filename and need to map back
     * to ID. Provided here to push down that part of refactoring. It
     * might be necessary to keep this method though because sometimes we
     * get filenames from the UI and need to convert it into our internal IDs.
     * If that turns out to be true, lowercasify this method, and update
     * this comment.
     */
    virtual DB::ResultId ID_FOR_FILE( const QString& ) const = 0; // QWERTY DIE ?

    /** @short Create a stack of images/videos/whatever
     *
     * If the specified images already belong to different stacks, then no
     * change happens and the function returns false.
     *
     * If some of them are in one stack and others aren't stacked at all, then
     * the unstacked will be added to the existing stack and we return true.
     *
     * If none of them are stacked, then a new stack is created and we return
     * true.
     *
     * All images which previously weren't in the stack are added in order they
     * are present in the provided list and after all items that are already in
     * the stack. The order of images which were already in the stack is not
     * changed.
     * */
    virtual bool stack(const DB::Result& items) = 0;

    /** @short Remove all images from whichever stacks they might be in
     *
     * We might destroy some stacks in the process if they become empty or just
     * containing one image.
     *
     * This function doesn't touch the order of images at all.
     * */
    virtual void unstack(const DB::Result& images) = 0;

    /** @short Return a list of images which are in the same stack as the one specified.
     *
     * Returns an empty list when the image is not stacked.
     *
     * They are returned sorted according to their stackOrder.
     * */
    virtual DB::Result getStackFor(const DB::ResultId& referenceId) const = 0;

 protected:
    friend class DB::ResultId;

    // Don't use directly, use DB::ResultId::fetchInfo() instead.
    virtual ImageInfoPtr info( const DB::ResultId& ) const = 0;


protected slots:
    virtual void lockDB( bool lock, bool exclude ) = 0;
    void markDirty();

signals:
    void totalChanged( uint );
    void dirty();
    void imagesDeleted( const DB::Result& );
};

}

#endif /* IMAGEDB_H */


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

#ifndef IMAGEDB_H
#define IMAGEDB_H

#include <QObject>

#include <DB/FileNameList.h>
#include <DB/ImageInfoList.h>
#include <DB/ImageInfoPtr.h>
#include <DB/MediaCount.h>
#include <DB/ImageDateCollection.h>

class QProgressBar;

namespace DB
{

class CategoryCollection;
class Category;
class MD5Map;
class MemberMap;
class ImageSearchInfo;
class FileName;

class ImageDB  :public QObject {
    Q_OBJECT

public:
    static ImageDB* instance();
    static void setupXMLDB( const QString& configFile );
    static void deleteInstance();

    DB::FileNameSet imagesWithMD5Changed();

public slots:
    void setDateRange( const ImageDate&, bool includeFuzzyCounts );
    void clearDateRange();
    virtual void slotRescan();
    void slotRecalcCheckSums(const DB::FileNameList& selection);
    virtual MediaCount count( const ImageSearchInfo& info );
    virtual void slotReread( const DB::FileNameList& list, DB::ExifMode mode);

protected:
    ImageDate m_selectionRange;
    bool m_includeFuzzyCounts;
    ImageInfoList m_clipboard;

private:
    static void connectSlots();
    static ImageDB* s_instance;

protected:
    ImageDB();

public:
    static QString NONE();
    DB::FileNameList currentScope(bool requireOnDisk) const;

    virtual DB::FileName findFirstItemInRange(
        const FileNameList& images,
        const ImageDate& range,
        bool includeRanges) const;

public: // Methods that must be overridden
    virtual uint totalCount() const = 0;
    virtual DB::FileNameList search(const ImageSearchInfo&, bool requireOnDisk=false) const = 0;

    virtual void renameCategory( const QString& oldName, const QString newName ) = 0;

    virtual QMap<QString,uint> classify( const ImageSearchInfo& info, const QString & category, MediaType typemask ) = 0;
    virtual FileNameList images() = 0;
    /**
     * @brief addImages to the database.
     * The parameter \p doUpdate decides whether all bookkeeping should be done right away
     * (\c true; the "normal" use-case), or if it should be deferred until later(\c false).
     * If doUpdate is deferred, either commitDelayedImages() or clearDelayedImages() needs to be called afterwards.
     * @param images
     * @param doUpdate
     */
    virtual void addImages( const ImageInfoList& images, bool doUpdate=true ) = 0;
    virtual void commitDelayedImages() = 0;
    virtual void clearDelayedImages() = 0;
    /** @short Update file name stored in the DB */
    virtual void renameImage( const ImageInfoPtr info, const DB::FileName& newName ) = 0;

    virtual void addToBlockList(const DB::FileNameList& list) = 0;
    virtual bool isBlocking( const DB::FileName& fileName ) = 0;
    virtual void deleteList(const DB::FileNameList& list) = 0;
    virtual ImageInfoPtr info( const DB::FileName& fileName ) const = 0;
    virtual MemberMap& memberMap() = 0;
    virtual void save( const QString& fileName, bool isAutoSave ) = 0;
    virtual MD5Map* md5Map() = 0;
    virtual void sortAndMergeBackIn(const DB::FileNameList& list) = 0;

    virtual CategoryCollection* categoryCollection() = 0;
    virtual QExplicitlySharedDataPointer<ImageDateCollection> rangeCollection() = 0;

    /**
     * Reorder the items in the database by placing all the items given in
     * cutList directly before or after the given item.
     * If the parameter "after" determines where to place it.
     */
    virtual void reorder(const DB::FileName& item, const DB::FileNameList& cutList, bool after) = 0;

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
    virtual bool stack(const DB::FileNameList& items) = 0;

    /** @short Remove all images from whichever stacks they might be in
     *
     * We might destroy some stacks in the process if they become empty or just
     * containing one image.
     *
     * This function doesn't touch the order of images at all.
     * */
    virtual void unstack(const DB::FileNameList& images) = 0;

    /** @short Return a list of images which are in the same stack as the one specified.
     *
     * Returns an empty list when the image is not stacked.
     *
     * They are returned sorted according to their stackOrder.
     * */
    virtual DB::FileNameList getStackFor(const DB::FileName& referenceId) const = 0;

    virtual void copyData( const DB::FileName& from, const DB::FileName& to) = 0;
protected slots:
    virtual void lockDB( bool lock, bool exclude ) = 0;
    void markDirty();

signals:
    void totalChanged( uint );
    void dirty();
    void imagesDeleted( const DB::FileNameList& );
};

}
#endif /* IMAGEDB_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

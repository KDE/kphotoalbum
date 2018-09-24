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

#ifndef XMLDB_DATABASE_H
#define XMLDB_DATABASE_H

#include "DB/ImageSearchInfo.h"
#include "DB/ImageInfoList.h"
#include <qstringlist.h>
#include "DB/MemberMap.h"
#include "DB/ImageDB.h"
#include "DB/Category.h"
#include "DB/CategoryCollection.h"
#include "XMLCategoryCollection.h"
#include "DB/MD5Map.h"
#include <qdom.h>
#include <DB/FileNameList.h>
#include "FileReader.h"

namespace DB
{
    class ImageInfo;
}

namespace XMLDB {
    class Database :public DB::ImageDB
    {
        Q_OBJECT

    public:
        uint totalCount() const override;
        DB::FileNameList search(
            const DB::ImageSearchInfo&,
            bool requireOnDisk=false) const override;
        void renameCategory( const QString& oldName, const QString newName ) override;

        QMap<QString,uint> classify( const DB::ImageSearchInfo& info, const QString &category, DB::MediaType typemask ) override;
        DB::FileNameList images() override;
        void addImages( const DB::ImageInfoList& images, bool doUpdate ) override;
        void commitDelayedImages() override;
        void clearDelayedImages() override;
        void renameImage( DB::ImageInfoPtr info, const DB::FileName& newName ) override;

        void addToBlockList(const DB::FileNameList& list) override;
        bool isBlocking( const DB::FileName& fileName ) override;
        void deleteList(const DB::FileNameList& list) override;
        DB::ImageInfoPtr info( const DB::FileName& fileName ) const override;
        DB::MemberMap& memberMap() override;
        void save( const QString& fileName, bool isAutoSave ) override;
        DB::MD5Map* md5Map() override;
        void sortAndMergeBackIn(const DB::FileNameList& idList) override;
        DB::CategoryCollection* categoryCollection() override;
        QExplicitlySharedDataPointer<DB::ImageDateCollection> rangeCollection() override;
        void reorder(
            const DB::FileName& item,
            const DB::FileNameList& cutList,
            bool after) override;

        static DB::ImageInfoPtr createImageInfo( const DB::FileName& fileName, ReaderPtr, Database* db = nullptr, const QMap<QString,QString> *newToOldCategory = nullptr );
        static void possibleLoadCompressedCategories( ReaderPtr reader , DB::ImageInfoPtr info, Database* db, const QMap<QString,QString> *newToOldCategory = nullptr );
        bool stack(const DB::FileNameList& items) override;
        void unstack(const DB::FileNameList& images) override;
        DB::FileNameList getStackFor(const DB::FileName& referenceId) const override;
        void copyData( const DB::FileName& from, const DB::FileName& to) override;

        static int fileVersion();
    protected:
        DB::FileNameList searchPrivate(
            const DB::ImageSearchInfo&,
            bool requireOnDisk,
            bool onlyItemsMatchingRange) const;
        bool rangeInclude( DB::ImageInfoPtr info ) const;

        DB::ImageInfoList takeImagesFromSelection(const DB::FileNameList& list);
        void insertList( const DB::FileName& id, const DB::ImageInfoList& list, bool after );
        static void readOptions( DB::ImageInfoPtr info, ReaderPtr reader, const QMap<QString,QString> *newToOldCategory = nullptr );


    protected slots:
        void renameItem( DB::Category* category, const QString& oldName, const QString& newName );
        void deleteItem( DB::Category* category, const QString& option );
        void lockDB( bool lock, bool exclude ) override;

    private:
        friend class DB::ImageDB;
        friend class FileReader;
        friend class FileWriter;

        Database( const QString& configFile );
        void forceUpdate( const DB::ImageInfoList& );

        QString m_fileName;
        DB::ImageInfoList m_images;
        QSet<DB::FileName> m_blockList;
        DB::ImageInfoList m_missingTimes;
        XMLCategoryCollection m_categoryCollection;
        DB::MemberMap m_members;
        DB::MD5Map m_md5map;
        //QMap<QString, QString> m_settings;

        DB::StackID m_nextStackId;
        typedef QMap<DB::StackID, DB::FileNameList> StackMap;
        mutable  StackMap m_stackMap;
        DB::ImageInfoList m_delayedUpdate;
	mutable QHash<const QString, DB::ImageInfoPtr> m_imageCache;
	mutable QHash<const QString, DB::ImageInfoPtr> m_delayedCache;

        // used for checking if any images are without image attribute from the database.
        static bool s_anyImageWithEmptySize;
    };
}

#endif /* XMLDB_DATABASE_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

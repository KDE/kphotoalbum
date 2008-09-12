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

#ifndef XMLDB_DATABSE_H
#define XMLDB_DATABSE_H
#include "IdNameMapper.h"
#include "DB/ImageSearchInfo.h"
#include "DB/ImageInfoList.h"
#include <qstringlist.h>
#include "DB/MemberMap.h"
#include "DB/ImageDB.h"
#include "DB/Category.h"
#include "DB/CategoryCollection.h"
#include "XMLCategoryCollection.h"
#include "DB/MD5Map.h"

namespace DB
{
    class ImageInfo;
}

namespace XMLDB {
    class Database :public DB::ImageDB
    {
        Q_OBJECT

    public:
        OVERRIDE bool operator==(const DB::ImageDB& other) const;
        OVERRIDE uint totalCount() const;
        OVERRIDE DB::ResultPtr search( const DB::ImageSearchInfo&, bool requireOnDisk = false ) const;
        OVERRIDE void renameCategory( const QString& oldName, const QString newName );

        OVERRIDE QMap<QString,uint> classify( const DB::ImageSearchInfo& info, const QString &group, DB::MediaType typemask );
        OVERRIDE DB::ResultPtr images();
        OVERRIDE void addImages( const DB::ImageInfoList& images );

        OVERRIDE void addToBlockList( const DB::ResultPtr& list );
        OVERRIDE bool isBlocking( const QString& fileName );
        OVERRIDE void deleteList( const DB::ResultPtr& list );
        OVERRIDE DB::ImageInfoPtr info( const QString& fileName, DB::PathType ) const;
        OVERRIDE DB::ImageInfoPtr info( const DB::ResultId& ) const;
        OVERRIDE DB::MemberMap& memberMap();
        OVERRIDE void save( const QString& fileName, bool isAutoSave );
        OVERRIDE DB::MD5Map* md5Map();
        OVERRIDE void sortAndMergeBackIn( const QStringList& fileList );
        OVERRIDE DB::CategoryCollection* categoryCollection();
        OVERRIDE KSharedPtr<DB::ImageDateCollection> rangeCollection();
        OVERRIDE void reorder( const DB::ResultId& item, const DB::ResultPtr& cutList, bool after );

        OVERRIDE void cutToClipboard( const QStringList& list );
        OVERRIDE QStringList pasteFromCliboard( const QString& afterFile );
         bool isClipboardEmpty();
        static DB::ImageInfoPtr createImageInfo( const QString& fileName, const QDomElement& elm, Database* db = 0 );
        static void possibleLoadCompressedCategories( const QDomElement& , DB::ImageInfoPtr info, Database* db );
        OVERRIDE bool stack( const DB::ResultPtr& items );
        OVERRIDE void unstack( const DB::ResultPtr& images );
        OVERRIDE DB::ResultPtr getStackFor( const DB::ResultId& referenceImg ) const;

        OVERRIDE QStringList CONVERT( const DB::ConstResultPtr& );
        OVERRIDE DB::ResultId ID_FOR_FILE( const QString& ) const;

    protected:
        DB::ResultPtr searchPrivate( const DB::ImageSearchInfo&, bool requireOnDisk, bool onlyItemsMatchingRange ) const;
        bool rangeInclude( DB::ImageInfoPtr info ) const;

        DB::ImageInfoList takeImagesFromSelection( const QStringList& list );
        QStringList insertList( const QString& fileName, const DB::ImageInfoList& list, bool after );
        static void readOptions( DB::ImageInfoPtr info, QDomElement elm );


    protected slots:
        void renameItem( DB::Category* category, const QString& oldName, const QString& newName );
        void deleteItem( DB::Category* category, const QString& option );
        void lockDB( bool lock, bool exclude );

    private:
        friend class DB::ImageDB;
        friend class FileReader;
        friend class FileWriter;

        Database( const QString& configFile );

        QString _fileName;
        DB::ImageInfoList _images;
        QStringList _blockList;
        DB::ImageInfoList _missingTimes;
        XMLCategoryCollection _categoryCollection;
        DB::MemberMap _members;
        DB::MD5Map _md5map;
        DB::ImageInfoList _clipboard;

        DB::StackID _nextStackId;
        typedef QMap<DB::StackID, DB::ResultPtr> StackMap;
        mutable  StackMap _stackMap;

        // used for checking if any images are without image attribute from the database.
        static bool _anyImageWithEmptySize;

        DB::IdNameMapper _idMapper;


        class StackSortHelper {
            const Database* const _db;
        public:
            StackSortHelper( const Database* const db );
            int operator()( const QString& a, const QString& b ) const;
        };
    };
}

#endif /* XMLDB_DATABSE_H */


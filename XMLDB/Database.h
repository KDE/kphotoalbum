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

#ifndef XMLDB_DATABSE_H
#define XMLDB_DATABSE_H
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
        virtual bool operator==(const DB::ImageDB& other) const;
        virtual uint totalCount() const;
        virtual QStringList search( const DB::ImageSearchInfo&, bool requireOnDisk = false ) const;
        virtual void renameCategory( const QString& oldName, const QString newName );

        virtual QMap<QString,uint> classify( const DB::ImageSearchInfo& info, const QString &group, DB::MediaType typemask );
        virtual QStringList images();
        virtual void addImages( const DB::ImageInfoList& images );

        virtual void addToBlockList( const QStringList& list );
        virtual bool isBlocking( const QString& fileName );
        virtual void deleteList( const QStringList& list );
        virtual DB::ImageInfoPtr info( const QString& fileName ) const;
        virtual DB::MemberMap& memberMap();
        virtual void save( const QString& fileName, bool isAutoSave );
        virtual DB::MD5Map* md5Map();
        virtual void sortAndMergeBackIn( const QStringList& fileList );
        virtual DB::CategoryCollection* categoryCollection();
        virtual KSharedPtr<DB::ImageDateCollection> rangeCollection();
        virtual void reorder( const QString& item, const QStringList& cutList, bool after );
        virtual void cutToClipboard( const QStringList& list );
        virtual QStringList pasteFromCliboard( const QString& afterFile );
        virtual bool isClipboardEmpty();
        static DB::ImageInfoPtr createImageInfo( const QString& fileName, const QDomElement& elm, Database* db = 0 );
        static void possibleLoadCompressedCategories( const QDomElement& , DB::ImageInfoPtr info, Database* db );


    protected:
        QStringList searchPrivate( const DB::ImageSearchInfo&, bool requireOnDisk, bool onlyItemsMatchingRange ) const;
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

        // used for checking if any images are without image attribute from the database.
        static bool _anyImageWithEmptySize;
};
}

#endif /* XMLDB_DATABSE_H */


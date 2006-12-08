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
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef SQLDB_DATABASE_H
#define SQLDB_DATABASE_H

#include "DB/ImageDB.h"
#include "DatabaseAddress.h"
#include "DatabaseHandler.h"
#include "SQLMemberMap.h"
#include "SQLCategoryCollection.h"
#include "SQLImageInfoCollection.h"
#include "SQLMD5Map.h"
#include "Connection.h"
#include "QueryHelper.h"

namespace SQLDB {
    class Database  :public DB::ImageDB {
        Q_OBJECT

    public:
        explicit Database(const DatabaseAddress& address);
        ~Database();

        virtual bool operator==(const DB::ImageDB& other) const;
        virtual uint totalCount() const;
        uint totalCount(DB::MediaType typemask) const;
        DB::MediaCount count(const DB::ImageSearchInfo& searchInfo);
        virtual QStringList search( const DB::ImageSearchInfo&, bool requireOnDisk = false ) const;

        virtual void renameCategory( const QString& oldName, const QString newName );

        virtual QMap<QString, uint> classify(const DB::ImageSearchInfo& info,
                                             const QString& category,
                                             DB::MediaType typemask);
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
        virtual QString
        findFirstItemInRange(const DB::ImageDate& range,
                             bool includeRanges,
                             const QValueVector<QString>& images) const;
        virtual void cutToClipboard( const QStringList& list );
        virtual QStringList pasteFromCliboard( const QString& afterFile );
        virtual bool isClipboardEmpty();

    protected slots:
        virtual void lockDB( bool lock, bool exclude );

    protected:
        QStringList imageList( bool withRelativePath );


    private:
        DatabaseAddress _address;
        DatabaseHandler _handler;
        Connection* _connection;
        QueryHelper _qh;
        SQLCategoryCollection _categoryCollection;
        SQLMemberMap _members;
        SQLImageInfoCollection _infoCollection;
        SQLMD5Map _md5map;

        // No copying or assignment
        Database(const Database&);
        Database& operator=(const Database&);
    };
}

#endif /* SQLDB_DATABASE_H */


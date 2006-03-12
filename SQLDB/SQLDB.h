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

#ifndef SQLDB_H
#define SQLDB_H

#include <imagedb.h>
#include <membermap.h>
#include <categorycollection.h>
#include <md5map.h>
#include "SQLCategoryCollection.h"
class QSqlError;

namespace SQLDB {
    class SQLDB  :public ImageDB {
        Q_OBJECT

    protected:
        friend class ImageDB;
        SQLDB();

    public:
        virtual int totalCount() const;
        virtual QStringList search( const ImageSearchInfo&, bool requireOnDisk = false ) const;

        virtual void renameCategory( const QString& oldName, const QString newName );

        virtual QMap<QString,int> classify( const ImageSearchInfo& info, const QString &group );
        virtual ImageInfoList& imageInfoList();
        virtual QStringList images();
        virtual void addImages( const ImageInfoList& images );

        virtual void addToBlockList( const QStringList& list );
        virtual bool isBlocking( const QString& fileName );
        virtual void deleteList( const QStringList& list );
        virtual ImageInfoPtr info( const QString& fileName ) const;
        virtual const MemberMap& memberMap();
        virtual void setMemberMap( const MemberMap& members );
        virtual void save( const QString& fileName, bool isAutoSave );
        virtual MD5Map* md5Map();
        virtual void sortAndMergeBackIn( const QStringList& fileList );
        virtual CategoryCollection* categoryCollection();
        virtual KSharedPtr<ImageDateCollection> rangeCollection();
        virtual void reorder( const QString& item, const QStringList& cutList, bool after );
        virtual void cutToClipboard( const QStringList& list );
        virtual QStringList pasteFromCliboard( const QString& afterFile );
        virtual bool isClipboardEmpty();

    protected slots:
        virtual void renameItem( Category* category, const QString& oldName, const QString& newName );
        virtual void deleteItem( Category* category, const QString& option );
        virtual void lockDB( bool lock, bool exclude );

    protected:
        void openDatabase();
        void loadMemberGroups();
        QStringList imageList( bool withRelativePath );

    private:
        SQLCategoryCollection _categoryCollection;
        MemberMap _members;
        MD5Map _md5map;
    };
}

#endif /* SQLDB_H */


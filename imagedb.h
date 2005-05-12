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

#ifndef IMAGEDB_H
#define IMAGEDB_H

#include "imagesearchinfo.h"
#include <qobject.h>
#include "imageinfolist.h"
#include "imageinfo.h"
class Category;
class MD5Map;
class ImageDateRange;
class MemberMap;

class ImageDB  :public QObject {
    Q_OBJECT

public:
    static ImageDB* instance();
    static void setup( const QString& configFile );
    ImageInfoList clipboard();
    void setClipboard( const ImageInfoList& list );
    bool isClipboardEmpty();

public slots:
    void setDateRange( const ImageDateRange&, bool includeFuzzyCounts );
    void clearDateRange();
    virtual void slotRescan();
    virtual void slotRecalcCheckSums();

protected:
    ImageDateRange _selectionRange;
    bool _includeFuzzyCounts;
    ImageInfoList _clipboard;

private:
    static ImageDB* _instance;

protected:
    ImageDB();

public:
    static QString NONE(); // OK
    ImageInfoList currentScope( bool requireOnDisk ) const; // OK

public: // Methods that must be overriden
    virtual int totalCount() const = 0; // OK
#ifdef TEMPORARILY_REMOVED
    virtual ImageInfoList search( const ImageSearchInfo& info, bool requireOnDisk = false ) const = 0; // OK
#endif
    virtual int count( const ImageSearchInfo& info ) = 0; //OK

    virtual void renameOptionGroup( const QString& oldName, const QString newName ) = 0;

    virtual QMap<QString,int> classify( const ImageSearchInfo& info, const QString &group ) = 0;
    virtual ImageInfoList& imageInfoList() = 0; // TO BE DELETED!
    virtual QStringList images() = 0; // TO BE REPLACED WITH URL's
    virtual void addImages( const ImageInfoList& images ) = 0; // OK

    virtual void addToBlockList( const ImageInfoList& list ) = 0; // OK
    virtual bool isBlocking( const QString& fileName ) = 0; // OK
    virtual void deleteList( const ImageInfoList& list ) = 0;
    virtual ImageInfo* info( const QString& fileName ) const = 0; // OK
    virtual const MemberMap& memberMap() = 0; // OK
    virtual void setMemberMap( const MemberMap& members ) = 0; // OK
    virtual void save( const QString& fileName ) = 0;
    virtual MD5Map* md5Map() = 0; // OK

protected slots:
    virtual void renameOption( Category* category, const QString& oldName, const QString& newName ) = 0;
    virtual void deleteOption( Category* category, const QString& option ) = 0;
    virtual void lockDB( bool lock, bool exclude ) = 0;

public slots:
    virtual void slotReread(ImageInfoList rereadList, int mode) = 0;

signals:
    void totalChanged( int );
    void dirty();
};


#endif /* IMAGEDB_H */


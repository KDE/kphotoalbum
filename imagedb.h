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
#include "imagedatecollection.h"
class CategoryCollection;
class Category;
class MD5Map;
class MemberMap;

class ImageDB  :public QObject {
    Q_OBJECT

public:
    static ImageDB* instance();
    static void setup( const QString& backEnd, const QString& configFile );
    void convertBackend();

public slots:
    void setDateRange( const ImageDate&, bool includeFuzzyCounts );
    void clearDateRange();
    virtual void slotRescan();
    virtual void slotRecalcCheckSums( QStringList selection );
    virtual int count( const ImageSearchInfo& info );
    virtual void slotReread( const QStringList& list, int mode);

protected:
    ImageDate _selectionRange;
    bool _includeFuzzyCounts;
    ImageInfoList _clipboard;

private:
    static ImageDB* _instance;

protected:
    ImageDB();

public:
    static QString NONE();
    QStringList currentScope( bool requireOnDisk ) const;

public: // Methods that must be overriden
    virtual int totalCount() const = 0;
    virtual QStringList search( const ImageSearchInfo&, bool requireOnDisk = false ) const = 0;

    virtual void renameCategory( const QString& oldName, const QString newName ) = 0;

    virtual QMap<QString,int> classify( const ImageSearchInfo& info, const QString & category ) = 0;
    virtual ImageInfoList& imageInfoList() = 0; // PENDING(blackie) TO BE DELETED!
    virtual QStringList images() = 0; // PENDING(blackie) TO BE REPLACED WITH URL's
    virtual void addImages( const ImageInfoList& images ) = 0;

    virtual void addToBlockList( const QStringList& list ) = 0;
    virtual bool isBlocking( const QString& fileName ) = 0;
    virtual void deleteList( const QStringList& list ) = 0;
    virtual ImageInfoPtr info( const QString& fileName ) const = 0;
    virtual const MemberMap& memberMap() = 0;
    virtual void setMemberMap( const MemberMap& members ) = 0;
    virtual void save( const QString& fileName, bool isAutoSave ) = 0;
    virtual MD5Map* md5Map() = 0;
    virtual void sortAndMergeBackIn( const QStringList& fileList ) = 0;
    virtual CategoryCollection* categoryCollection() = 0;
    virtual KSharedPtr<ImageDateCollection> rangeCollection() = 0;

    virtual void reorder( const QString& item, const QStringList& cutList, bool after ) = 0;
    virtual void cutToClipboard( const QStringList& list ) = 0;
    virtual QStringList pasteFromCliboard( const QString& afterFile ) = 0;
    virtual bool isClipboardEmpty() = 0;

protected slots:
    virtual void renameItem( Category* category, const QString& oldName, const QString& newName ) = 0;
    virtual void deleteItem( Category* category, const QString& option ) = 0;
    virtual void lockDB( bool lock, bool exclude ) = 0;


signals:
    void totalChanged( int );
    void dirty();
};


#endif /* IMAGEDB_H */


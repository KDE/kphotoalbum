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
class ImageDateRange;
class MemberMap;

class ImageDB  :public QObject {
    Q_OBJECT

public:
    static ImageDB* instance();
    static bool setup( const QString& configFile );

private:
    static ImageDB* _instance;

public:
    static QString NONE();

    virtual int totalCount() const = 0; // OK
    virtual ImageInfoList search( const ImageSearchInfo& info ) = 0; // OK
    virtual int count( const ImageSearchInfo& info ) = 0; //OK

    virtual int countItemsOfCategory( const QString& group ) = 0;
    virtual void renameOptionGroup( const QString& oldName, const QString newName ) = 0;

    virtual QMap<QString,int> classify( const ImageSearchInfo& info, const QString &group ) = 0;
    virtual ImageInfoList& images() = 0;
    virtual ImageInfoList images( const ImageSearchInfo& info, bool onDisk ) = 0;
    virtual void addImage( ImageInfo* info ) = 0;
    virtual ImageInfoList& clipboard() = 0;
    virtual bool isClipboardEmpty() = 0;

    virtual void blockList( const ImageInfoList& list ) = 0;
    virtual void deleteList( const ImageInfoList& list ) = 0;
    virtual void showUnavailableImages() = 0;
    virtual ImageInfoList currentContext( bool onDisk ) const = 0;
    virtual ImageInfo* find( const QString& fileName ) const = 0;
    virtual const MemberMap& memberMap() = 0;
    virtual void setMemberMap( const MemberMap& members ) = 0;
    virtual void save( const QString& fileName ) = 0;

public slots:
    virtual void slotRescan() = 0;
    virtual void slotRecalcCheckSums() = 0;
    virtual void slotReread(ImageInfoList rereadList, int mode) = 0;
    virtual void setDateRange( const ImageDateRange&, bool includeFuzzyCounts ) = 0;
    virtual void clearDateRange() = 0;

signals:
    void totalChanged( int );
    void dirty();
};


#endif /* IMAGEDB_H */


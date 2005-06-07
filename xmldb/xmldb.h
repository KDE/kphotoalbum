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

#ifndef XMLDB_H
#define XMLDB_H
#include "imagesearchinfo.h"
#include <qdict.h>
#include "imageinfolist.h"
#include <qobject.h>
#include <qstringlist.h>
#include "imagedaterange.h"
#include "membermap.h"
#include <imagedb.h>
#include <category.h>
#include "newimagefinder.h"
#include <categorycollection.h>
#include "xmlcategorycollection.h"
class ImageInfo;

namespace XMLDB {
    class XMLDB :public ImageDB
    {
        Q_OBJECT
    public:
        virtual int totalCount() const;
        virtual QStringList search( const ImageSearchInfo&, bool requireOnDisk = false ) const;
        virtual void renameCategory( const QString& oldName, const QString newName );

        virtual QMap<QString,int> classify( const ImageSearchInfo& info, const QString &group );
        virtual ImageInfoList& imageInfoList() { return _images; }
        virtual QStringList images();
        virtual void addImages( const ImageInfoList& images );

        virtual void addToBlockList( const QStringList& list );
        virtual bool isBlocking( const QString& fileName );
        virtual void deleteList( const QStringList& list );
        virtual ImageInfo* info( const QString& fileName ) const;
        virtual const MemberMap& memberMap();
        virtual void setMemberMap( const MemberMap& members );
        virtual void save( const QString& fileName );
        virtual MD5Map* md5Map();
        virtual void sortAndMergeBackIn( const QStringList& fileList );
        virtual CategoryCollection* categoryCollection();

    public slots:
        void slotReread( const QStringList& list, int mode);

    protected:
        ImageInfo* load( const QString& filename, QDomElement elm );
        void checkIfImagesAreSorted();
        bool rangeInclude( ImageInfo* info ) const;
        void checkIfAllImagesHasSizeAttributes();

        QDomElement readConfigFile( const QString& configFile );
        void readTopNodeInConfigDocument( const QString& configFile, QDomElement top, QDomElement* options, QDomElement* images,
                                          QDomElement* blockList, QDomElement* memberGroups );
        void loadCategories( const QDomElement& elm );
        void loadImages( const QDomElement& images );
        void loadBlockList( const QDomElement& blockList );
        void loadMemberGroups( const QDomElement& memberGroups );

        void saveImages( QDomDocument doc, QDomElement top );
        void saveBlockList( QDomDocument doc, QDomElement top );
        void saveMemberGroups( QDomDocument doc, QDomElement top );
        void saveCategories( QDomDocument doc, QDomElement top );


    protected slots:
        void renameItem( Category* category, const QString& oldName, const QString& newName );
        void deleteItem( Category* category, const QString& option );
        void lockDB( bool lock, bool exclude );

    private:
        friend class ImageDB;
        XMLDB( const QString& configFile );

        ImageInfoList _images;
        QStringList _blockList;
        ImageInfoList _missingTimes;
        MemberMap _members;
        MD5Map _md5map;
        XMLCategoryCollection _categoryCollection;
    };
}

#endif /* XMLDB_H */


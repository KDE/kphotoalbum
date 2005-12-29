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
#include "XMLCategoryCollection.h"
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
        virtual ImageInfoPtr info( const QString& fileName ) const;
        virtual const MemberMap& memberMap();
        virtual void setMemberMap( const MemberMap& members );
        virtual void save( const QString& fileName, bool isAutoSave );
        virtual MD5Map* md5Map();
        virtual void sortAndMergeBackIn( const QStringList& fileList );
        virtual CategoryCollection* categoryCollection();
        virtual KSharedPtr<ImageDateRangeCollection> rangeCollection();
        virtual void reorder( const QString& item, const QStringList& cutList, bool after );
        virtual void cutToClipboard( const QStringList& list );
        virtual QStringList pasteFromCliboard( const QString& afterFile );
        virtual bool isClipboardEmpty();
        int fileVersion();
        static ImageInfoPtr createImageInfo( const QString& fileName, const QDomElement& elm, XMLDB* db = 0 );
        static void possibleLoadCompressedCategories( const QDomElement& , ImageInfoPtr info, XMLDB* db );


    protected:
        ImageInfoPtr load( const QString& filename, QDomElement elm );
        void checkIfImagesAreSorted();
        bool rangeInclude( ImageInfoPtr info ) const;
        void checkIfAllImagesHasSizeAttributes();

        QDomElement readConfigFile( const QString& configFile );
        void readTopNodeInConfigDocument( const QString& configFile, QDomElement top, QDomElement* options, QDomElement* images,
                                          QDomElement* blockList, QDomElement* memberGroups );
        void loadCategories( const QDomElement& elm );
        void createSpecialCategories();
        void loadImages( const QDomElement& images );
        void loadBlockList( const QDomElement& blockList );
        void loadMemberGroups( const QDomElement& memberGroups );

        void saveImages( QDomDocument doc, QDomElement top );
        void saveBlockList( QDomDocument doc, QDomElement top );
        void saveMemberGroups( QDomDocument doc, QDomElement top );
        void saveCategories( QDomDocument doc, QDomElement top );
        ImageInfoList takeImagesFromSelection( const QStringList& list );
        QStringList insertList( const QString& fileName, const ImageInfoList& list, bool after );
        void add21CompatXML( QDomElement& top );
        static void readOptions( ImageInfoPtr info, QDomElement elm );


    protected slots:
        void renameItem( Category* category, const QString& oldName, const QString& newName );
        void deleteItem( Category* category, const QString& option );
        void lockDB( bool lock, bool exclude );
        void checkAndWarnAboutVersionConflict();

    private:
        friend class ImageDB;
        XMLDB( const QString& configFile );

        ImageInfoList _images;
        QStringList _blockList;
        ImageInfoList _missingTimes;
        XMLCategoryCollection _categoryCollection;
        MemberMap _members;
        MD5Map _md5map;
        ImageInfoList _clipboard;
        int _fileVersion;

        // used for checking if any images are without image attribute from the database.
        static bool _anyImageWithEmptySize;
};
}

#endif /* XMLDB_H */


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
class ImageInfo;

class XMLDB :public ImageDB
{
    Q_OBJECT
public:
    int totalCount() const;
    ImageInfoList search( const ImageSearchInfo& info, bool requireOnDisk = false ) const;
    int count( const ImageSearchInfo& info );
    void renameOptionGroup( const QString& oldName, const QString newName );

    QMap<QString,int> classify( const ImageSearchInfo& info, const QString &group );
    ImageInfoList& images() { return _images; }
    void addImage( ImageInfo* info );

    void blockList( const ImageInfoList& list );
    void deleteList( const ImageInfoList& list );
    ImageInfo* find( const QString& fileName ) const;
    const MemberMap& memberMap();
    void setMemberMap( const MemberMap& members );
    void save( const QString& fileName );

public slots:
    void slotRescan();
    void slotRecalcCheckSums();
    void slotReread(ImageInfoList rereadList, int mode);

protected:
    void save( QDomElement top );
    void searchForNewFiles( const QDict<void>& loadedFiles, QString directory );
    void loadExtraFiles();
    void mergeNewImagesInWithExistingList( ImageInfoList newImages );
    ImageInfo* loadExtraFile( const QString& name );
    ImageInfo* load( const QString& filename, QDomElement elm );
    bool calculateMD5sums( ImageInfoList& list );
    QString MD5Sum( const QString& fileName );
    QDict<void> findAlreadyMatched( const ImageSearchInfo& info, const QString &group );
    void checkIfImagesAreSorted();
    bool rangeInclude( ImageInfo* info ) const;
    void checkIfAllImagesHasSizeAttributes();
    void loadOptions( const QDomElement& elm );
    void saveOptions( QDomElement top );

protected slots:
    void renameOption( Category* category, const QString& oldName, const QString& newName );
    void deleteOption( Category* category, const QString& option );
    void lockDB( bool lock, bool exclude );

private:
    friend class ImageDB;
    XMLDB( const QString& configFile, bool* newImages );

    ImageInfoList _images;
    QStringList _blockList;
    ImageInfoList _missingTimes;
    QMap<QString, QString> _md5Map;
    QMap<QString, ImageInfo* > _fileMap;
    QStringList _pendingLoad;
    MemberMap _members;
};


#endif /* XMLDB_H */


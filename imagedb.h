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
#include <qdict.h>
#include "imageinfolist.h"
#include <qobject.h>
#include <qstringlist.h>
#include "imagedaterange.h"
class ImageInfo;

class ImageDB :public QObject {
    Q_OBJECT

public:
    static ImageDB* instance();
    static bool setup( const QDomElement& images, const QDomElement& blockList);

    int totalCount() const;
    void search( const ImageSearchInfo& info, int from = -1, int to = -1 );
    int count( const ImageSearchInfo& info );
    int countItemsOfCategory( const QString& group );
    void renameOptionGroup( const QString& oldName, const QString newName );

    QMap<QString,int> classify( const ImageSearchInfo& info, const QString &group );
    ImageInfoList& images() { return _images; }
    ImageInfoList images( const ImageSearchInfo& info, bool onDisk );
    void addImage( ImageInfo* info );
    ImageInfoList& clipboard() { return _clipboard; }
    bool isClipboardEmpty();

    void blockList( const ImageInfoList& list );
    void deleteList( const ImageInfoList& list );
    void showUnavailableImages();
    ImageInfoList currentContext( bool onDisk ) const;
    ImageInfo* find( const QString& fileName ) const;

public slots:
    void save( QDomElement top );
    void slotRescan();
    void slotRecalcCheckSums();
    void slotReread(ImageInfoList rereadList, int mode);
    void setDateRange( const ImageDateRange&, bool includeFuzzyCounts );
    void clearDateRange();

signals:
    void matchCountChange( int, int, int );
    void totalChanged( int );
    void searchCompleted();
    void dirty();

protected:
    void searchForNewFiles( const QDict<void>& loadedFiles, QString directory );
    void loadExtraFiles();
    void mergeNewImagesInWithExistingList( ImageInfoList newImages );
    ImageInfo* loadExtraFile( const QString& name );
    ImageInfo* load( const QString& filename, QDomElement elm );
    int count( const ImageSearchInfo& info, bool makeVisible, int from, int to );
    bool calculateMD5sums( ImageInfoList& list );
    QString MD5Sum( const QString& fileName );
    QDict<void> findAlreadyMatched( const ImageSearchInfo& info, const QString &group );
    void checkIfImagesAreSorted();
    bool rangeInclude( ImageInfo* info );
    void checkIfAllImagesHasSizeAttributes();

protected slots:
    void renameOption( const QString& category, const QString& oldName, const QString& newName );
    void deleteOption( const QString& category, const QString& option );
    void lockDB( bool lock, bool exclude );

private:
    ImageDB( const QDomElement& images, const QDomElement& blockList, bool* newImages );
    static ImageDB* _instance;

    ImageInfoList _images;
    QStringList _blockList;
    ImageInfoList _clipboard, _missingTimes;
    QMap<QString, QString> _md5Map;
    QMap<QString, ImageInfo* > _fileMap;
    QStringList _pendingLoad;
    ImageDateRange _selectionRange;
    bool _includeFuzzyCounts;

public:
    static QString NONE();

};


#endif /* IMAGEDB_H */


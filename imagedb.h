/*
 *  Copyright (c) 2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#ifndef IMAGEDB_H
#define IMAGEDB_H
#include "imageinfo.h"
#include "imagesearchinfo.h"
#include <qdict.h>

class ImageDB :public QObject {
    Q_OBJECT

public:
    static ImageDB* instance();
    static bool setup( const QDomElement& images, const QDomElement& blockList);

    int totalCount() const;
    void search( const ImageSearchInfo& info, int from = -1, int to = -1 );
    int count( const ImageSearchInfo& info );
    int countItemsOfOptionGroup( const QString& group );
    void renameOptionGroup( const QString& oldName, const QString newName );

    QMap<QString,int> classify( const ImageSearchInfo& info, const QString &group );
    ImageInfoList& images() { return _images; }
    ImageInfoList& clipboard() { return _clipboard; }
    bool isClipboardEmpty();

    void blockList( const ImageInfoList& list );
    void deleteList( const ImageInfoList& list );

public slots:
    void save( QDomElement top );

signals:
    void matchCountChange( int, int, int );
    void searchCompleted();

protected:
    void loadExtraFiles( const QDict<void>& loadedFiles, QString directory );
    void load( const QString& filename, QDomElement elm );
    int count( const ImageSearchInfo& info, bool makeVisible, int from, int to );

protected slots:
    void renameOption( const QString& optionGroup, const QString& oldName, const QString& newName );
    void deleteOption( const QString& optionGroup, const QString& option );
    void lockDB( bool lock, bool exclude );

private:
    ImageDB( const QDomElement& images, const QDomElement& blockList, bool* newImages );
    static ImageDB* _instance;

    ImageInfoList _images;
    QStringList _blockList;
    ImageInfoList _clipboard;
};


#endif /* IMAGEDB_H */


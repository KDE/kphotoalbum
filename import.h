/*
 *  Copyright (c) 2003-2004 Jesper K. Pedersen <blackie@kde.org>
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

#ifndef IMPORT_H
#define IMPORT_H

#include <kwizard.h>
#include "imageinfo.h"
class Import;
class ImageInfo;
class QCheckBox;
class KArchiveDirectory;
class KZip;
class ImportMatcher;
class KLineEdit;

class ImageRow :public QObject
{
    Q_OBJECT
public:
    ImageRow( ImageInfo* info, Import* import, QWidget* parent );
    QCheckBox* _checkbox;
    ImageInfo* _info;
    Import* _import;
protected slots:
    void showImage();
};

class Import :public KWizard {
    Q_OBJECT

public:
    static void imageImport( QString file );

protected:
    friend class ImageRow;

    void setupPages();
    bool readFile( const QByteArray& data, const QString& fileName );
    void createIntroduction();
    void createImagesPage();
    void createDestination();
    void createOptionPages();
    ImportMatcher* createOptionPage( const QString& myOptionGroup, const QString& otherOptionGroup );
    QMap<QString,QString> copyFilesFromZipFile();
    QPixmap loadThumbnail( QString fileName );
    QByteArray loadImage( const QString& fileName );
    void selectImage( bool on );
    ImageInfoList selectedImages();

protected slots:
    void slotEditDestination();
    void updateNextButtonState();
    virtual void next();
    void slotFinish();
    void slotSelectAll();
    void slotSelectNone();

private:
    Import( const QString& file, bool* ok, QWidget* parent, const char* name = 0 );
    ~Import();
    QString _zipFile;
    ImageInfoList _images;
    KLineEdit* _destinationEdit;
    QWidget* _destinationPage;
    QWidget* _dummy;
    ImportMatcher* _optionGroupMatcher;
    QValueList<ImportMatcher*> _matchers;
    KZip* _zip;
    const KArchiveDirectory* _dir;
    QValueList< ImageRow* > _imagesSelect;
};



#endif /* IMPORT_H */


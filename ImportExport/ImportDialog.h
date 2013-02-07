/* Copyright (C) 2003-2010 Jesper K. Pedersen <blackie@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IMPORT_H
#define IMPORT_H

#include "ImportSettings.h"
#include <kurl.h>
#include <KAssistantDialog>
#include "ImportMatcher.h"

class KTemporaryFile;
class KLineEdit;

namespace DB
{
    class ImageInfo;
}

namespace ImportExport
{
class ImportMatcher;
class ImageRow;
class KimFileReader;
class MD5CheckPage;

/**
 * This is the wizard that configures the import process
 */
class ImportDialog :public KAssistantDialog {
    Q_OBJECT

public:
    ImportDialog( QWidget* parent );
    bool exec( KimFileReader* kimFileReader, const QString& fileName, const KUrl& kimFilePath );
    ImportSettings settings();

protected:
    friend class ImageRow;

    void setupPages();
    bool readFile( const QByteArray& data, const QString& fileName );
    void createIntroduction();
    void createImagesPage();
    void createDestination();
    void createCategoryPages();
    ImportMatcher* createCategoryPage( const QString& myCategory, const QString& otherCategory );
    void selectImage( bool on );
    DB::ImageInfoList selectedImages() const;
    void possiblyAddMD5CheckPage();

protected slots:
    void slotEditDestination();
    void updateNextButtonState();
    virtual void next();
    void slotSelectAll();
    void slotSelectNone();
    void slotHelp();

signals:
    void failedToCopy( QStringList files );

private:
    DB::ImageInfoList _images;
    KLineEdit* _destinationEdit;
    KPageWidgetItem* _destinationPage;
    KPageWidgetItem* _categoryMatcherPage;
    KPageWidgetItem* _dummy;
    ImportMatcher* _categoryMatcher;
    ImportMatchers _matchers;
    QList< ImageRow* > _imagesSelect;
    KTemporaryFile* _tmp;
    bool _externalSource;
    KUrl _kimFile;
    bool _hasFilled;
    QString _baseUrl;
    KimFileReader* _kimFileReader;
    MD5CheckPage* _md5CheckPage;
};

}


#endif /* IMPORT_H */

// vi:expandtab:tabstop=4 shiftwidth=4:

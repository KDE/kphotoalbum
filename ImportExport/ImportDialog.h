/* Copyright (C) 2003-2006 Jesper K. Pedersen <blackie@kde.org>

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

#include "ImportHandler.h"
#include <kurl.h>
#include <kio/job.h>
#include "Utilities/Util.h" // JKP
#include <QPixmap>
#include <Q3ValueList>
#include <QCloseEvent>
#include <KAssistantDialog>

class KTemporaryFile;
class QCheckBox;
class KArchiveDirectory;
class KZip;
class KLineEdit;
class QProgressDialog;

namespace DB
{
    class ImageInfo;
}

namespace ImportExport
{
class ImportMatcher;
class ImageRow;

class ImportDialog :public KAssistantDialog {
    Q_OBJECT

public:
    friend class ImportHandler;
    static void imageImport();
    static void imageImport( const KUrl& url );

protected:
    friend class ImageRow;

    void setupPages();
    bool readFile( const QByteArray& data, const QString& fileName );
    void createIntroduction();
    void createImagesPage();
    void createDestination();
    void createCategoryPages();
    ImportMatcher* createCategoryPage( const QString& myCategory, const QString& otherCategory );
    QPixmap loadThumbnail( QString fileName );
    QByteArray loadImage( const QString& fileName );
    void selectImage( bool on );
    DB::ImageInfoList selectedImages();
    bool init( const QString& fileName );
    virtual void closeEvent( QCloseEvent* );

protected slots:
    void slotEditDestination();
    void updateNextButtonState();
    virtual void next();
    void slotFinish();
    void slotSelectAll();
    void slotSelectNone();
    void downloadKimJobCompleted( KIO::Job* );
    void slotHelp();

signals:
    void failedToCopy( QStringList files );

private:
    ImportDialog( const QString& file, bool* ok, QWidget* parent );
    ImportDialog( const KUrl& url, QWidget* parent );
    ~ImportDialog();

    QString _zipFile;
    DB::ImageInfoList _images;
    KLineEdit* _destinationEdit;
    KPageWidgetItem* _destinationPage;
    KPageWidgetItem* _categoryMatcherPage;
    KPageWidgetItem* _dummy;
    ImportMatcher* _categoryMatcher;
    QList<ImportMatcher*> _matchers;
    KZip* _zip;
    const KArchiveDirectory* _dir;
    QList< ImageRow* > _imagesSelect;
    KTemporaryFile* _tmp;
    bool _externalSource;
    KUrl _kimFile;
    bool _hasFilled;
    QString _baseUrl;
    ImportHandler m_importHandler;
};

}


#endif /* IMPORT_H */


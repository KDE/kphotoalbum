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
    static void imageImport();

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
    QPixmap loadThumbnail( const QString& fileName );
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


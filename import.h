#ifndef IMPORT_H
#define IMPORT_H

#include <kwizard.h>
#include "imageinfo.h"
class ImportMatcher;
class KLineEdit;

class Import :public KWizard {
    Q_OBJECT

public:
    static void imageImport();

protected:
    void setupPages();
    bool readFile( const QByteArray& data, const QString& fileName );
    void createIntroduction();
    void createDestination();
    void createOptionPages();
    ImportMatcher* createOptionPage( const QString& myOptionGroup, const QString& otherOptionGroup );
    QMap<QString,QString> copyFilesFromZipFile();

protected slots:
    void slotEditDestination();
    void updateNextButtonState();
    virtual void next();
    void slotFinish();

private:
    Import( const QString& file, bool* ok, QWidget* parent, const char* name = 0 );
    QString _zipFile;
    ImageInfoList _images;
    KLineEdit* _destinationEdit;
    QWidget* _destinationPage;
    QWidget* _dummy;
    ImportMatcher* _optionGroupMatcher;
    QValueList<ImportMatcher*> _matchers;
};


#endif /* IMPORT_H */


#ifndef IMAGEROW_H
#define IMAGEROW_H

#include "DB/ImageInfoPtr.h"
#include <QObject>

class QCheckBox;
namespace ImportExport
{
class ImportDialog;
class KimFileReader;

/**
 * This class represent a single row on the ImageDialog's "select widgets to import" page.
 */
class ImageRow :public QObject
{
    Q_OBJECT
public:
    ImageRow( DB::ImageInfoPtr info, ImportDialog* import, KimFileReader* kimFileReader,QWidget* parent );
    QCheckBox* m_checkbox;
    DB::ImageInfoPtr m_info;
    ImportDialog* m_import;
    KimFileReader* m_kimFileReader;

protected slots:
    void showImage();
};

}

#endif /* IMAGEROW_H */


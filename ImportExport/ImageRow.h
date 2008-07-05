#ifndef IMAGEROW_H
#define IMAGEROW_H

#include "DB/ImageInfoPtr.h"
#include <QObject>

class QCheckBox;
namespace ImportExport
{
class ImportDialog;

class ImageRow :public QObject
{
    Q_OBJECT
public:
    ImageRow( DB::ImageInfoPtr info, ImportExport::ImportDialog* import, QWidget* parent );
    QCheckBox* _checkbox;
    DB::ImageInfoPtr _info;
    ImportDialog* _import;

protected slots:
    void showImage();
};

}

#endif /* IMAGEROW_H */


#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H
#include "imageinfo.h"
#include <kdialogbase.h>
class QLabel;
class QCheckBox;
class QRadioButton;

class DeleteDialog :public KDialogBase {
    Q_OBJECT

public:
    DeleteDialog( QWidget* parent, const char* name = 0 );
    void exec( const ImageInfoList& );

protected slots:
    void deleteImages();

private:
    ImageInfoList _list;
    QLabel* _label;
    QCheckBox* _deleteFromDisk;
    QCheckBox* _block;

};


#endif /* DELETEDIALOG_H */


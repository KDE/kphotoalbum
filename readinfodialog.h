#ifndef READINFODIALOG_H
#define READINFODIALOG_H
#include "imageinfo.h"
#include <kdialogbase.h>
class QLabel;
class QCheckBox;
class QRadioButton;

class ReadInfoDialog :public KDialogBase {
    Q_OBJECT

public:
    ReadInfoDialog( QWidget* parent, const char* name = 0 );
    int exec( const ImageInfoList& );

protected slots:
    void readInfo();
private:
    ImageInfoList _list;
    QLabel* _label;
    QCheckBox* _time;
    QCheckBox* _date;
    QCheckBox* _orientation;
    QCheckBox* _description;
};


#endif /* READINFODIALOG_H */


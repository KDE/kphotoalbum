#ifndef HTMLEXPORTDIALOG_H
#define HTMLEXPORTDIALOG_H
#include <kdialogbase.h>
class KLineEdit;
class QSpinBox;
class QCheckBox;
class QProgressDialog;
class QSlider;
class MyCheckBox;
#include "imageinfo.h"
#include "imageclient.h"
#include <qvaluelist.h>

class HTMLExportDialog :public KDialogBase, private ImageClient {
    Q_OBJECT

public:
    HTMLExportDialog( const ImageInfoList& list, QWidget* parent, const char* name = 0 );

protected slots:
    void slotOk();
    void selectDir();
    void slotCancelGenerate();

protected:
    QString createImage( ImageInfo* info, int size );
    QString imageName( const QString& fileName, int size );
    virtual void pixmapLoaded( const QString& fileName, int width, int height, int angle, const QImage& );
    bool generate();
    QString outputDir( bool showErr );

private:
    KLineEdit* _title;
    KLineEdit* _baseDir;
    KLineEdit* _baseURL;
    KLineEdit* _outputDir;
    QSpinBox* _thumbSize;
    QCheckBox* _generateToolTips;
    QSlider* _numOfCols;
    QValueList<MyCheckBox*> _cbs;

    ImageInfoList _list;
    int _waitCounter;
    int _total;
    QProgressDialog* _progress;
    bool _doneLoading;
};


#endif /* HTMLEXPORTDIALOG_H */


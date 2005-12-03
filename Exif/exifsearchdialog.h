#ifndef EXIFSEARCHDIALOG_H
#define EXIFSEARCHDIALOG_H

#include <kdialogbase.h>
#include <qvaluelist.h>
#include "ExifSearchInfo.h"
class QCheckBox;

class IntValueSetting
{
public:
    IntValueSetting() {}
    IntValueSetting( QCheckBox* cb, int value ) : cb( cb ), value( value ) {}
    QCheckBox* cb;
    int value;
};

class Settings :public QValueList<IntValueSetting>
{
public:
    QValueList<int> selected();
};

class ExifSearchDialog : public KDialogBase {
    Q_OBJECT

public:
    ExifSearchDialog( QWidget* parent, const char* name = 0 );
    ExifSearchInfo info();

protected:
    QWidget* makeISO( QWidget* parent );
    QWidget* makeExposureProgram( QWidget* parent );
    QWidget* makeOrientation( QWidget* parent );
    QWidget* makeMeteringMode( QWidget* parent );
    QWidget* makeContrast( QWidget* parent );
    QWidget* makeSharpness( QWidget* parent );
    QWidget* makeSaturation( QWidget* parent );
    QWidget* makeCamera( QWidget* parent );
    QWidget* makeExposureTime( QWidget* parent );
    QWidget* makeFNumber( QWidget* parent );

    QStringList availableCameras();

private:
    Settings _iso;
    Settings _exposureProgram;
    Settings _orientation;
    Settings _meteringMode;
    Settings _contrast;
    Settings _sharpness;
    Settings _saturation;
    Settings _cameras;
};


#endif /* EXIFSEARCHDIALOG_H */


#ifndef EXIFSEARCHDIALOG_H
#define EXIFSEARCHDIALOG_H

#include <kdialogbase.h>
#include <qvaluelist.h>
#include "Exif/SearchInfo.h"
#include "Exif/SearchDialogSettings.h"
#include "Exif/RangeWidget.h"

class QCheckBox;

namespace Exif
{

class SearchDialog : public KDialogBase {
    Q_OBJECT

public:
    SearchDialog( QWidget* parent, const char* name = 0 );
    Exif::SearchInfo info();

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
    Exif::RangeWidget* _iso;
    Exif::RangeWidget* _exposureTime;
    Exif::RangeWidget* _fnumber;
    IntSettings _exposureProgram;
    IntSettings _orientation;
    IntSettings _meteringMode;
    IntSettings _contrast;
    IntSettings _sharpness;
    IntSettings _saturation;
    IntSettings _cameras;
};

}

#endif /* EXIFSEARCHDIALOG_H */

